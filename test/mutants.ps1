# Mutation check for rend's geometry tests.
#
# WHY THIS EXISTS: a test suite that passes tells you nothing until you have watched it fail
# for the right reasons. The first version of test_rend_draw.c passed cleanly against three
# separate broken implementations -- a missing corner arc, a corner displaced by a whole
# pixel, and every fill span a pixel too wide -- because its tolerances were round numbers
# picked by eye rather than measured against what the code actually achieves.
#
# Each mutant below is a plausible way of breaking rend that compiles cleanly. Every one
# should make ctest go red. A mutant that PASSES marks a blind spot: either add a case that
# catches it, or convince yourself the mutated code is genuinely equivalent (one here is --
# see the notes at the end).
#
# HOW IT WORKS: patches src/rend_draw.c in place, rebuilds, runs ctest, restores. The source
# is always restored, including on Ctrl-C, but it IS edited in your working tree while this
# runs -- do not run it with uncommitted changes to that file.
#
# USAGE:  pwsh test/mutants.ps1 [-BuildDir <path>]
param(
        [string]$BuildDir = (Join-Path $PSScriptRoot '..\..\..\build-host')
)

$ErrorActionPreference = 'Stop'
$src = Join-Path $PSScriptRoot '..\src\rend_draw.c'
$src = (Resolve-Path $src).Path

if (-not (Test-Path $BuildDir)) {
        Write-Error "no build directory at $BuildDir -- configure a HOST build first, or pass -BuildDir"
}
$BuildDir = (Resolve-Path $BuildDir).Path

# (name, search, replace). Single-line search strings only: the file on disk may be CRLF while
# a PowerShell literal is LF, so a multi-line search silently never matches -- which looks
# exactly like a mutant that was caught.
$mutants = @(
 @('half the arc samples',         '>> 24) * 2 + 1;', '>> 24) + 1;'),
 @('accumulate the angle',         'int32_t theta = start_q8 + (int32_t)(((int64_t)span_q8 * i) / steps);', 'int32_t theta = start_q8 + i * (span_q8 / steps);'),
 @('no sine interpolation',        'return s0 + (((s1 - s0) * frac) >> 8);', 'return s0 + 0*(s1+frac);'),
 @('corner arc shifted 1px',       'if(tl) _draw_arc(ctx, (rend_point2d) { (uint16_t)(x0 + r), (uint16_t)(y0 + r) },', 'if(tl) _draw_arc(ctx, (rend_point2d) { (uint16_t)(x0 + r + 1), (uint16_t)(y0 + r) },'),
 @('fill span 1px too wide',       '_set_span_clipped(ctx, cx - px, cx + px, cy + py, color);', '_set_span_clipped(ctx, cx - px, cx + px + 1, cy + py, color);'),
 @('drop coordinate rounding',     '(((int32_t)radius * _cos_deg_q8_q15(theta) + 16384) >> 15)', '(((int32_t)radius * _cos_deg_q8_q15(theta)) >> 15)'),
 @('arc sweeps the short way',     'if(span <= 0) span += 360;', 'if(span < 0) span = -span;'),
 @('circle fill ignored',          '_set_spans_circle(ctx, p, radius, ctx->color_fg);', '_set_pixels_circle(ctx, p, radius, ctx->color_fg);'),
 @('corner mask ignored',          'int32_t tl = (corners & REND_CORNER_TOP_LEFT)     ? r : 0;', 'int32_t tl = r;')
)

$original = Get-Content $src -Raw
$restored = $false
function Restore-Source {
        if (-not $script:restored) {
                Set-Content $script:src -Value $script:original -NoNewline
                $script:restored = $true
                Write-Host "source restored"
        }
}
# fires on Ctrl-C and on an uncaught error alike, so a half-finished run never leaves mutated
# code in the tree
trap { Restore-Source; break }

try {
        Write-Host "=== baseline (unmutated) ==="
        & cmake --build $BuildDir --target test_rend_draw 2>&1 | Out-Null
        & ctest --test-dir $BuildDir -R '^rend\.' 2>&1 | Select-Object -Last 1

        Write-Host "`n=== mutants (each SHOULD fail) ==="
        foreach ($m in $mutants) {
                $name, $find, $replace = $m
                $patched = $original.Replace($find, $replace)
                if ($patched -eq $original) {
                        "{0,-28} !! search string not found -- has rend_draw.c moved on?" -f $name
                        continue
                }
                Set-Content $src -Value $patched -NoNewline
                $build = & cmake --build $BuildDir --target test_rend_draw 2>&1
                if ($LASTEXITCODE -ne 0) {
                        "{0,-28} -- did not compile (not a useful mutant)" -f $name
                } else {
                        $out = & ctest --test-dir $BuildDir -R '^rend\.' 2>&1
                        $line = ($out | Select-String 'tests passed' | Select-Object -First 1)
                        $verdict = if ($LASTEXITCODE -ne 0) { 'caught' } else { '*** SURVIVED ***' }
                        "{0,-28} -> {1}  ({2})" -f $name, $verdict, ($line -replace '\s+', ' ').Trim()
                }
                Set-Content $src -Value $original -NoNewline
        }
} finally {
        Restore-Source
        & cmake --build $BuildDir --target test_rend_draw 2>&1 | Out-Null
}

# KNOWN SURVIVOR: 'half the arc samples'. rend_draw_arc() samples at half-pixel spacing so the
# no-gap guarantee holds through coordinate rounding -- two samples a whole pixel apart CAN
# round to pixels two apart. At one-pixel spacing that worst case is not reached by any radius
# or span this suite sweeps, so the mutant passes. The 2x factor is a proof-carrying margin
# rather than an empirically necessary one, and it is cheap; do not remove it on the strength
# of this surviving.
