import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root

    property var skill: ({})
    property var palette
    property var helpProvider
    property var diceRoller
    property var resourceAdjuster
    property var resourceResetter
    property var resourceCurrentSetter
    property var resourceMaxSetter
    property var resourceAdjusterAt
    property var resourceCurrentSetterAt
    property var resourceMaxSetterAt
    property bool resourceEnabled: !!skill.id && !skill.isLocked

    clip: true

    ColumnLayout {
        width: root.availableWidth
        spacing: 10

        Label {
            text: skill.name || "Select a skill"
            color: palette.textPrimary
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: skill.isLocked ? "Locked" : "Unlocked"
            color: skill.isLocked ? palette.textSecondary : palette.accent
            font.pixelSize: 12
        }

        Text {
            text: formatInteractiveText(skill.shortDescription || "", skill.isLocked)
            textFormat: Text.RichText
            color: palette.textSecondary
            linkColor: palette.accentSoft
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            onLinkActivated: handleLink(link)
        }

        Rectangle {
            Layout.fillWidth: true
            color: palette.panelAltBg
            border.width: 1
            border.color: palette.inputBorder
            radius: 8
            implicitHeight: resourceColumn.implicitHeight + 14

            ColumnLayout {
                id: resourceColumn
                anchors.fill: parent
                anchors.margins: 7
                spacing: 6

                Label {
                    text: "Resources"
                    color: palette.textPrimary
                    font.bold: true
                }

                Repeater {
                    model: skill.resources || []
                    delegate: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: modelData.name
                                color: palette.textSecondary
                                Layout.fillWidth: true
                            }
                            FlatButton { text: "-"; palette: root.palette; onClicked: resourceAdjusterAt(index, -1); enabled: root.resourceEnabled }
                            TextField {
                                Layout.preferredWidth: 70
                                text: String(modelData.current ?? 0)
                                validator: IntValidator { bottom: 0 }
                                onEditingFinished: resourceCurrentSetterAt(index, Number(text))
                                enabled: root.resourceEnabled
                            }
                            Label {
                                text: (modelData.max ?? -1) >= 0 ? "/" + String(modelData.max) : ""
                                color: palette.textSecondary
                            }
                            TextField {
                                Layout.preferredWidth: 70
                                visible: (modelData.max ?? -1) >= 0
                                text: String(modelData.max ?? -1)
                                validator: IntValidator { bottom: 0 }
                                onEditingFinished: resourceMaxSetterAt(index, Number(text))
                                enabled: root.resourceEnabled
                            }
                            FlatButton { text: "+"; palette: root.palette; onClicked: resourceAdjusterAt(index, 1); enabled: root.resourceEnabled }
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: (modelData.max ?? -1) >= 0 ? modelData.max : Math.max(1, modelData.current ?? 1)
                            value: modelData.current ?? 0
                            enabled: root.resourceEnabled
                        }
                    }
                }
            }
        }

        Text {
            width: parent.width
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            color: palette.textPrimary
            linkColor: palette.accentSoft
            text: formatInteractiveText(skill.longDescription || "", skill.isLocked)
            onLinkActivated: handleLink(link)
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: palette.inputBorder
        }

        Label {
            text: "Custom Fields"
            color: palette.textPrimary
            font.pixelSize: 16
            font.bold: true
        }

        Repeater {
            model: {
                const m = skill.customFields || {}
                const out = []
                for (const k in m) {
                    out.push({ key: k, value: m[k] })
                }
                return out
            }
            delegate: Label {
                Layout.fillWidth: true
                text: modelData.key + ": " + modelData.value
                color: palette.textSecondary
                wrapMode: Text.Wrap
            }
        }
    }

    Popup {
        id: helpPopup
        x: Math.max(10, root.width * 0.1)
        y: Math.max(10, root.height * 0.22)
        width: Math.min(400, root.width - 40)
        modal: false
        padding: 12
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 100 } }
        exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 80 } }

        background: Rectangle {
            radius: 8
            color: palette.tooltipBg
            border.color: palette.inputBorder
            border.width: 1
        }

        Label {
            id: helpPopupText
            width: parent.width
            wrapMode: Text.Wrap
            color: palette.tooltipText
            text: ""
        }
    }

    function formatInteractiveText(src, lockedState) {
        let t = src.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
        if (lockedState) {
            t = t.replace(/(\b\d*d\d+(?:[+-]\d*d?\d+)*\b)/g, '<span style="color:' + palette.textSecondary + ';">$1</span>')
        } else {
            t = t.replace(/(\b\d*d\d+(?:[+-]\d*d?\d+)*\b)/g, '<a href="dice:$1" style="text-decoration: underline; color:' + palette.accent + ';">$1</a>')
        }
        t = t.replace(/\[([^\]]+)\]/g, '<a href="help:$1" style="text-decoration:none;border-bottom:1px dotted ' + palette.accentSoft + ';color:' + palette.accentSoft + ';">[$1]</a>')
        return t.replace(/\n/g, "<br>")
    }

    function handleLink(link) {
        if (link.startsWith("dice:")) {
            diceRoller(link.substring(5))
            return
        }
        if (link.startsWith("help:")) {
            const term = link.substring(5)
            const text = helpProvider(term)
            helpPopupText.text = text.length > 0 ? text : "No help entry for '" + term + "'"
            helpPopup.open()
        }
    }
}
