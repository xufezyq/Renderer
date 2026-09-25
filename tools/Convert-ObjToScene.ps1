param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath,

    [float]$Scale = 0.01,

    [switch]$MirrorX
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Format-Float([double]$Value) {
    return $Value.ToString('0.######', [Globalization.CultureInfo]::InvariantCulture)
}

function Parse-Index([string]$Token, [int]$VertexCount) {
    $vertexToken = ($Token -split '/')[0]
    $index = [int]$vertexToken
    if ($index -lt 0) {
        return $VertexCount + $index
    }
    return $index - 1
}

function Format-Vertex([double[]]$Vertex, [bool]$Mirror) {
    $x = $Vertex[0]
    if ($Mirror) {
        $x = -$x
    }
    return ((@($x, $Vertex[1], $Vertex[2]) | ForEach-Object { Format-Float $_ }) -join ' ')
}

$vertices = [System.Collections.Generic.List[double[]]]::new()
$faces = [System.Collections.Generic.List[int[]]]::new()
$skippedTriangles = 0

foreach ($line in [System.IO.File]::ReadLines($InputPath)) {
    if ($line.StartsWith('v ')) {
        $parts = $line.Trim() -split '\s+'
        if ($parts.Count -lt 4) {
            throw "Invalid vertex line: $line"
        }
        $vertices.Add(@(
            [double]::Parse($parts[1], [Globalization.CultureInfo]::InvariantCulture),
            [double]::Parse($parts[2], [Globalization.CultureInfo]::InvariantCulture),
            [double]::Parse($parts[3], [Globalization.CultureInfo]::InvariantCulture)))
    }
    elseif ($line.StartsWith('f ')) {
        $parts = $line.Trim() -split '\s+'
        if ($parts.Count -lt 4) {
            continue
        }
        $face = [int[]]::new($parts.Count - 1)
        for ($i = 1; $i -lt $parts.Count; ++$i) {
            $face[$i - 1] = Parse-Index $parts[$i] $vertices.Count
            if ($face[$i - 1] -lt 0 -or $face[$i - 1] -ge $vertices.Count) {
                throw "Face references an invalid vertex: $line"
            }
        }
        $faces.Add($face)
    }
}

$writer = [System.IO.StreamWriter]::new(
    $OutputPath,
    $false,
    [System.Text.UTF8Encoding]::new($false))
try {
    $writer.WriteLine('<?xml version="1.0" encoding="UTF-8"?>')
    $writer.WriteLine('<!-- 由 OBJ 模型生成；面已三角化，未导入材质。 -->')
    if ($MirrorX) {
        $writer.WriteLine('<!-- 已沿 X 轴镜像模型，并修正三角形顶点顺序。 -->')
    }
    $writer.WriteLine('<scene version="2">')
    $writer.WriteLine('    <camera')
    $writer.WriteLine('        position="0 36 -4"')
    $writer.WriteLine('        forward="0 -0.98 0.11"')
    $writer.WriteLine('        fov="75" />')
    $writer.WriteLine('    <objects>')
    $writer.WriteLine('        <object scale="' + (Format-Float $Scale) + '">')

    foreach ($face in $faces) {
        for ($i = 1; $i -lt ($face.Count - 1); ++$i) {
            $a = $vertices[$face[0]]
            $b = $vertices[$face[$i]]
            $c = $vertices[$face[$i + 1]]
            $abx = $b[0] - $a[0]
            $aby = $b[1] - $a[1]
            $abz = $b[2] - $a[2]
            $acx = $c[0] - $a[0]
            $acy = $c[1] - $a[1]
            $acz = $c[2] - $a[2]
            $crossX = $aby * $acz - $abz * $acy
            $crossY = $abz * $acx - $abx * $acz
            $crossZ = $abx * $acy - $aby * $acx
            # 为渲染器的单精度三角形计算保留误差余量。
            if (($crossX * $crossX + $crossY * $crossY + $crossZ * $crossZ) -le 1.0e-4) {
                ++$skippedTriangles
                continue
            }
            $vertex0 = $a
            $vertex1 = $b
            $vertex2 = $c
            if ($MirrorX) {
                $vertex1 = $c
                $vertex2 = $b
            }
            $v0 = Format-Vertex $vertex0 $MirrorX
            $v1 = Format-Vertex $vertex1 $MirrorX
            $v2 = Format-Vertex $vertex2 $MirrorX
            $writer.WriteLine(('            <triangle v0="{0}" v1="{1}" v2="{2}" />' -f $v0, $v1, $v2))
        }
    }

    $writer.WriteLine('        </object>')
    $writer.WriteLine('    </objects>')
    $writer.WriteLine('</scene>')
}
finally {
    $writer.Dispose()
}

Write-Output "Converted $($vertices.Count) vertices and $($faces.Count) faces to $OutputPath; skipped $skippedTriangles degenerate triangles"
