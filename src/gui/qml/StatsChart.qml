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
import QtQuick 2.2
import "StatsChart.js" as Chart
import "items/ToolTip.js" as ToolTipCreator

Item {
    id: root

    // TODO: Use a model!
    property var stats

    property bool verticalGrid: true
    property int horizontalGridLines: 5
    property int dotRadius: 5
    property int xSkip: 40
    property int maxRate: 360
    property int minRate: 90

    function update() {
        canvas.requestPaint()
    }

    onStatsChanged: canvas.updatePoints()
    onXSkipChanged: canvas.updatePoints()
    onMaxRateChanged: canvas.updatePoints()
    onMinRateChanged: canvas.updatePoints()

    Canvas {
        id: canvas

        anchors.fill: parent

        property var points: []
        property int highestRate: 0
        property int lowestRate: 0

        function updatePoints() {
            var i = 0

            // Destroy old ToolTips
            for (i = 0; i < points.length; i++) {
                points[i].accuracy.tooltip.destroy()
                points[i].rate.tooltip.destroy()
            }
            points = []

            highestRate = root.maxRate
            lowestRate = root.minRate
            for (i = 0; i < stats.length; i++) {
                var s = stats[i]

                var hits = Math.max(s.chars - s.errors, 0)
                var acc = 100 / s.chars * hits
                var rate = s.chars / (s.time / 1000 / 60)

                console.log("Stat:", i + 1, "of", stats.length, "Accuracy:",
                            acc.toFixed(2), "Rate:", rate)

                if (parseFloat(highestRate) < parseFloat(rate))
                    highestRate = rate
                if (lowestRate < 0 || parseFloat(lowestRate) > parseFloat(rate))
                    lowestRate = rate

                var x = i * root.xSkip + dotRadius

                // Note that the tooltip has no partent. The reference is hold in points property.
                points.push({
                                accuracy: {
                                    value: acc,
                                    x: x,
                                    tooltip: ToolTipCreator.create({
                                                                       text: qsTr("Accuracy") + "\n" + acc.toFixed(2),
                                                                       parent: canvas,
                                                                       x: x
                                                                   })
                                },
                                rate: {
                                    value: rate,
                                    x: x,
                                    tooltip: ToolTipCreator.create({
                                                                       text: qsTr("Rate") + "\n" + rate,
                                                                       parent: canvas,
                                                                       x: x
                                                                   })
                                }
                            })
            } // for

            requestPaint()
        }

        onPaint: {
            if (!root.stats || stats.length === 0) {
                return
            }

            console.debug("Repaint")

            var ctx = getContext("2d")
            ctx.globalCompositeOperation = "source-over"

            ctx.clearRect(0, 0, width, height)

            ctx.lineWidth = 1

            Chart.drawGrid(ctx, width, height, "#d7d7d7",
                           horizontalGridLines)

            // Update all values that depend on the current item dimensions
            for (var i = 0; i < points.length; i++) {
                var p = points[i]
                p.accuracy.y = height - (p.accuracy.value / 100 * height)
                p.accuracy.tooltip.y = height - (p.accuracy.value / 100 * height)

                p.rate.y = height * (lowestRate - p.rate.value)
                        / (highestRate - lowestRate) + height
                p.rate.tooltip.y = height * (lowestRate - p.rate.value)
                        / (highestRate - lowestRate) + height
            }

            Chart.drawLines(ctx, points, "accuracy", "red")
            Chart.drawDots(ctx, points, "accuracy", "red", dotRadius)
            Chart.drawLines(ctx, points, "rate", "black")
            Chart.drawDots(ctx, points, "rate", "black", dotRadius)
        } // onPaint
    } // Canvas
} // root
