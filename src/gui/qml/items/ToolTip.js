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

// The tooltip component is based on an implementation from Xander84
// found on https://wiki.qt.io/QtQuick_ToolTip_Component
var component = Qt.createComponent("ToolTip.qml")

function create(properties) {
    // NOTE: Returned object has no parent so do not loose the reference.
    var tooltip = component.createObject(null, properties)
    if (tooltip === null) {
        console.error("Error creating tooltip:", component.errorString())
    } else if (properties.anchors) {
        // manual anchor mapping necessary
        for (var anchor in properties.anchors) {
            tooltip.anchors[anchor] = properties.anchors[anchor]
        }
    }
    return tooltip
}
