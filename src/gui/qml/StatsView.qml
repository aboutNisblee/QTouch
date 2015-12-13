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
import QtQuick 2.0

Item {
    id: root

    property alias stats: chart.stats

    property alias verticalGrid: chart.verticalGrid
    property alias horizontalGridLines: chart.horizontalGridLines
    property alias dotRadius: chart.dotRadius
    property alias xSkip: chart.xSkip
    property alias maxRate: chart.maxRate
    property alias minRate: chart.minRate

    function update() {
        chart.update()
    }

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: chart.width
        contentHeight: chart.height
        flickableDirection: Flickable.HorizontalFlick
        StatsChart {
            id: chart
            width: Math.max(
                       (root.stats
                        && root.stats.length > 0) ? (root.stats.length - 1) * root.xSkip
                                                    + 2 * root.dotRadius : 0,
                                                    root.width)
            height: root.height
        }
    }
}
