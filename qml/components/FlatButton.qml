import QtQuick
import QtQuick.Controls

Button {
    id: root
    property var palette

    implicitHeight: 32
    padding: 10

    contentItem: Label {
        text: root.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: root.enabled ? palette.textPrimary : palette.textSecondary
        font.pixelSize: 13
        font.weight: Font.Medium
    }

    background: Rectangle {
        radius: 7
        border.width: 1
        border.color: root.down ? palette.accentSoft : palette.inputBorder
        color: {
            if (!root.enabled) return palette.panelAltBg
            if (root.down) return palette.menuHoverBg
            if (root.hovered) return palette.menuHoverBg
            return palette.panelAltBg
        }

        Behavior on color { ColorAnimation { duration: 90 } }
        Behavior on border.color { ColorAnimation { duration: 90 } }
    }
}
