import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Item {
    id: root

    property string skillId: ""
    property string label: ""
    property string iconPath: ""
    property string shortDescription: ""
    property bool selected: false
    property bool locked: false
    property bool collapsed: false
    property var palette

    signal clicked(string skillId)
    signal contextRequested(string skillId, real localX, real localY)
    signal collapseToggled(string skillId)
    signal linkActivated(string skillId, string link)

    width: 128
    height: 150

    Rectangle {
        id: glow
        anchors.centerIn: orb
        width: 104
        height: 104
        radius: width / 2
        color: root.selected ? (palette.nodeSelectedGlow || palette.accent) : (palette.nodeGlow || palette.accentSoft)
        opacity: root.selected ? 0.22 : (mouseArea.containsMouse ? 0.14 : 0.0)
        scale: mouseArea.containsMouse ? 1.05 : 1.0
        Behavior on opacity { NumberAnimation { duration: 120 } }
        Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
    }

    Rectangle {
        id: orb
        anchors.horizontalCenter: parent.horizontalCenter
        y: 0
        width: 88
        height: 88
        radius: 44
        clip: true
        color: root.locked ? palette.nodeLockedFill : (mouseArea.containsMouse ? palette.nodeHoverFill : palette.nodeFill)
        border.width: root.selected ? 2 : 1
        border.color: root.selected ? palette.nodeSelectedBorder : palette.nodeBorder
        scale: mouseArea.containsMouse ? 1.03 : 1.0

        Behavior on color { ColorAnimation { duration: 120 } }
        Behavior on border.color { ColorAnimation { duration: 120 } }
        Behavior on scale { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }

        Item {
            anchors.fill: parent
            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: orb.width
                    height: orb.height
                    radius: orb.radius
                }
            }

            Image {
                id: iconImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                source: root.iconPath.length > 0 ? root.iconPath : "qrc:/icons/default_skill.svg"
                smooth: true
            }

            Rectangle {
                anchors.fill: parent
                color: root.locked ? (palette.nodeLockedOverlay || "#66000000") : "#12000000"
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 20
            height: 20
            radius: 10
            visible: root.locked
            color: palette.panelBg
            border.width: 1
            border.color: palette.inputBorder

            Label {
                anchors.centerIn: parent
                text: "L"
                color: palette.textSecondary
                font.pixelSize: 10
                font.bold: true
            }
        }

        Rectangle {
            width: 18
            height: 18
            radius: 9
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 2
            color: palette.panelBg
            border.width: 1
            border.color: palette.inputBorder

            Label {
                anchors.centerIn: parent
                text: root.collapsed ? "+" : "-"
                color: palette.textSecondary
                font.pixelSize: 12
                font.bold: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.collapseToggled(root.skillId)
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: mouseArea.containsMouse ? 1 : 0
            border.color: palette.accentSoft
            opacity: 0.35
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: 2
            border.color: palette.nodeSelectedBorder
            opacity: root.selected ? 0.7 : 0.0
        }
    }

    SequentialAnimation on opacity {
        running: root.selected
        loops: Animation.Infinite
        NumberAnimation { to: 0.92; duration: 900; easing.type: Easing.InOutSine }
        NumberAnimation { to: 1.0; duration: 900; easing.type: Easing.InOutSine }
    }

    Label {
        anchors.top: orb.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
        text: root.label
        color: root.locked ? palette.textSecondary : palette.textPrimary
        font.pixelSize: 13
        font.weight: root.selected ? Font.DemiBold : Font.Normal
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                root.contextRequested(root.skillId, mouse.x, mouse.y)
                return
            }
            root.clicked(root.skillId)
        }
    }

    Timer {
        id: tooltipDelay
        interval: 180
        onTriggered: {
            if (mouseArea.containsMouse && root.shortDescription.length > 0) {
                nodeTooltip.open()
            }
        }
    }

    Popup {
        id: nodeTooltip
        x: orb.x + orb.width + 10
        y: Math.max(0, orb.y - 4)
        width: Math.min(300, 320)
        modal: false
        focus: false
        padding: 10
        closePolicy: Popup.NoAutoClose

        enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 100 } }
        exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 90 } }

        background: Rectangle {
            radius: 8
            color: palette.tooltipBg
            border.width: 1
            border.color: palette.inputBorder
        }

        Text {
            width: parent.width
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            color: palette.tooltipText
            linkColor: palette.accent
            text: formatRichText(root.shortDescription, root.locked)
            onLinkActivated: function(link) {
                root.linkActivated(root.skillId, link)
            }
        }
    }

    function formatRichText(src, lockedState) {
        let t = (src || "").replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
        if (lockedState) {
            t = t.replace(/(\b\d*d\d+(?:[+-]\d*d?\d+)*\b)/g, '<span style="color:' + palette.textSecondary + ';">$1</span>')
        } else {
            t = t.replace(/(\b\d*d\d+(?:[+-]\d*d?\d+)*\b)/g, '<a href="dice:$1" style="text-decoration: underline; color:' + palette.accent + ';">$1</a>')
        }
        t = t.replace(/\[([^\]]+)\]/g, '<a href="help:$1" style="text-decoration:none;border-bottom:1px dotted ' + palette.accentSoft + ';color:' + palette.accentSoft + ';">[$1]</a>')
        return t.replace(/\n/g, "<br>")
    }

    onVisibleChanged: {
        if (!visible) {
            tooltipDelay.stop()
            nodeTooltip.close()
        }
    }

    Connections {
        target: mouseArea
        function onContainsMouseChanged() {
            if (mouseArea.containsMouse) {
                tooltipDelay.restart()
            } else {
                tooltipDelay.stop()
                nodeTooltip.close()
            }
        }
    }
}
