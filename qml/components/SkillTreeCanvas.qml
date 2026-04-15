import QtQuick
import QtQuick.Controls

Item {
    id: root

    property var nodes: []
    property var edges: []
    property var palette
    property var helpProvider
    property var diceRoller
    signal skillSelected(string skillId)
    signal skillContextMenuRequested(string skillId, real globalX, real globalY)
    signal skillCollapseToggled(string skillId)
    signal helpRequested(string term)

    clip: true

    Rectangle { anchors.fill: parent; color: palette.treeBg || palette.windowBg }

    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: scene.width * zoom
        contentHeight: scene.height * zoom
        clip: true

        property real zoom: 1.0
        property bool initialized: false

        Item {
            id: scene
            width: 2200
            height: 1600
            scale: flick.zoom
            transformOrigin: Item.TopLeft

            Canvas {
                id: gridCanvas
                anchors.fill: parent
                antialiasing: false

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    const spacing = 72
                    ctx.lineWidth = 1
                    ctx.strokeStyle = palette.treeGridColor || palette.inputBorder
                    ctx.globalAlpha = 0.18
                    for (let x = 0; x <= width; x += spacing) {
                        ctx.beginPath()
                        ctx.moveTo(x + 0.5, 0)
                        ctx.lineTo(x + 0.5, height)
                        ctx.stroke()
                    }
                    for (let y = 0; y <= height; y += spacing) {
                        ctx.beginPath()
                        ctx.moveTo(0, y + 0.5)
                        ctx.lineTo(width, y + 0.5)
                        ctx.stroke()
                    }
                }
            }

            Canvas {
                id: branchCanvas
                anchors.fill: parent
                antialiasing: true

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()

                    for (let i = 0; i < root.edges.length; ++i) {
                        const e = root.edges[i]
                        const active = !!e.active
                        const x1 = scene.width / 2 + e.x1
                        const y1 = 110 + e.y1
                        const x2 = scene.width / 2 + e.x2
                        const y2 = 110 + e.y2

                        ctx.lineWidth = active ? (root.palette.edgeActiveWidth || 3.6) : (root.palette.edgeWidth || 2.8)
                        ctx.strokeStyle = active ? (root.palette.edgeActiveColor || root.palette.accent) : root.palette.edgeColor
                        ctx.globalAlpha = active ? 0.95 : 0.75
                        ctx.beginPath()
                        ctx.moveTo(x1, y1)
                        const midY = (y1 + y2) / 2
                        ctx.bezierCurveTo(x1, midY, x2, midY, x2, y2)
                        ctx.stroke()
                    }
                }
            }

            Repeater {
                model: root.nodes

                delegate: SkillNodeItem {
                    x: scene.width / 2 + modelData.x - width / 2
                    y: 60 + modelData.y
                    skillId: modelData.id
                    label: modelData.name
                    iconPath: modelData.iconPath
                    shortDescription: modelData.shortDescription
                    selected: modelData.selected
                    locked: modelData.isLocked
                    collapsed: modelData.isCollapsed
                    palette: root.palette

                    onClicked: function(id) {
                        root.skillSelected(id)
                    }

                    onContextRequested: function(id, localX, localY) {
                        const g = mapToGlobal(localX, localY)
                        root.skillContextMenuRequested(id, g.x, g.y)
                    }

                    onCollapseToggled: function(id) {
                        root.skillCollapseToggled(id)
                    }

                    onLinkActivated: function(id, link) {
                        if (link.startsWith("dice:")) {
                            root.diceRoller(id, link.substring(5))
                            return
                        }
                        if (link.startsWith("help:")) {
                            root.helpRequested(link.substring(5))
                        }
                    }
                }
            }
        }

        WheelHandler {
            acceptedDevices: PointerDevice.Mouse
            onWheel: function(event) {
                if (event.angleDelta.y > 0) {
                    flick.zoom = Math.min(2.3, flick.zoom * 1.1)
                } else {
                    flick.zoom = Math.max(0.45, flick.zoom / 1.1)
                }
                event.accepted = true
            }
        }

        function centerOnTree() {
            contentX = Math.max(0, (contentWidth - width) / 2)
            contentY = 0
        }

        Component.onCompleted: {
            centerOnTree()
            initialized = true
        }

        onWidthChanged: {
            if (initialized) {
                centerOnTree()
            }
        }
    }

    Connections {
        target: root
        function onEdgesChanged() { branchCanvas.requestPaint() }
        function onNodesChanged() { branchCanvas.requestPaint() }
        function onPaletteChanged() { gridCanvas.requestPaint(); branchCanvas.requestPaint() }
    }
}
