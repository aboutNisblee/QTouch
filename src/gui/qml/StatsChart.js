/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
**
** This file is part of QTouch.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/

function drawGrid(ctx, width, height, color, horizontalGridLines) {
    ctx.save()
    ctx.strokeStyle = color
    ctx.beginPath()

    // Horizontal grid lines
    for (var i = 1; i <= horizontalGridLines; i++) {
        ctx.moveTo(0, i * (height / (horizontalGridLines + 1)))
        ctx.lineTo(width, i * (height / (horizontalGridLines + 1)))
    }

    ctx.stroke()
    ctx.restore()
}

function drawLines(ctx, points, which, color) {
    ctx.save()
    ctx.globalAlpha = 0.7
    ctx.strokeStyle = color
    ctx.lineWidth = 3
    ctx.beginPath()

    for (var i = 0; i < points.length; i++) {
        var x = points[i][which].x
        var y = points[i][which].y

        if (i == 0) {
            ctx.moveTo(x, y)
        } else {
            ctx.lineTo(x, y)
        }
    }
    ctx.lineJoin = 'round'
    ctx.stroke()
    ctx.restore()
}

function drawDots(ctx, points, which, color, dotRadius) {
    ctx.save()
    ctx.fillStyle = color
    ctx.beginPath()

    for (var i = 0; i < points.length; i++) {
        var x = points[i][which].x
        var y = points[i][which].y

        ctx.moveTo(x, y)
        ctx.arc(x, y, dotRadius, 0, 2 * Math.PI)
    }
    ctx.fill()
    ctx.restore()
}
