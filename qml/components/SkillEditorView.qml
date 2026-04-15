import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ScrollView {
    id: root

    property var skill: ({})
    property var palette
    property var helpEntriesMap: ({})
    property string saveError: ""

    signal saveRequested(var payload)
    signal cancelRequested()

    ColumnLayout {
        width: root.availableWidth
        spacing: 10

        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: "Skill Name"
            text: skill.name || ""
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: "ID"; color: palette.textSecondary }
            TextField {
                id: idField
                Layout.fillWidth: true
                placeholderText: "Unique ID (auto-generated if empty)"
                text: skill.id || ""
                readOnly: (skill.id || "").length > 0
            }
        }

        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: iconField
                Layout.fillWidth: true
                placeholderText: "Icon path or qrc"
                text: skill.iconPath || ""
            }
            FlatButton {
                text: "Choose Icon"
                palette: root.palette
                onClicked: iconDialog.open()
            }
        }

        Rectangle {
            Layout.preferredWidth: 60
            Layout.preferredHeight: 60
            radius: 8
            color: palette.panelAltBg
            border.color: palette.inputBorder
            border.width: 1
            Image {
                anchors.fill: parent
                anchors.margins: 8
                fillMode: Image.PreserveAspectFit
                source: iconField.text.length > 0 ? iconField.text : "qrc:/icons/default_skill.svg"
            }
        }

        CheckBox {
            id: lockedCheck
            text: "Locked"
            checked: skill.isLocked !== undefined ? skill.isLocked : true
        }

        Label { text: "Resources"; color: palette.textPrimary; font.bold: true }

        ListModel { id: resourcesModel }

        Repeater {
            model: resourcesModel
            delegate: RowLayout {
                Layout.fillWidth: true
                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Resource name"
                    text: model.name
                    onTextChanged: resourcesModel.setProperty(index, "name", text)
                }
                TextField {
                    Layout.preferredWidth: 78
                    text: String(model.current)
                    validator: IntValidator { bottom: 0 }
                    onTextChanged: resourcesModel.setProperty(index, "current", Number(text))
                }
                TextField {
                    Layout.preferredWidth: 78
                    text: String(model.max)
                    validator: IntValidator { bottom: -1 }
                    onTextChanged: resourcesModel.setProperty(index, "max", Number(text))
                }
                FlatButton { text: "-"; palette: root.palette; onClicked: resourcesModel.setProperty(index, "current", Math.max(0, Number(resourcesModel.get(index).current) - 1)) }
                FlatButton { text: "+"; palette: root.palette; onClicked: resourcesModel.setProperty(index, "current", Number(resourcesModel.get(index).current) + 1) }
                FlatButton { text: "Del"; palette: root.palette; onClicked: resourcesModel.remove(index) }
            }
        }

        FlatButton {
            text: "Add Resource"
            palette: root.palette
            onClicked: resourcesModel.append({ name: "Resource", current: 0, max: -1 })
        }

        TextArea {
            id: shortField
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            placeholderText: "Short description"
            text: skill.shortDescription || ""
            wrapMode: TextArea.Wrap
        }

        TextArea {
            id: longField
            Layout.fillWidth: true
            Layout.preferredHeight: 170
            placeholderText: "Long description (dice: 1d20+2, help: [Strength])"
            text: skill.longDescription || ""
            wrapMode: TextArea.Wrap
        }

        Label { text: "Custom Fields"; color: palette.textPrimary; font.bold: true }

        ListModel { id: customFieldsModel }

        Repeater {
            model: customFieldsModel
            delegate: RowLayout {
                Layout.fillWidth: true
                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Key"
                    text: model.key
                    onTextChanged: customFieldsModel.setProperty(index, "key", text)
                }
                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Value"
                    text: model.value
                    onTextChanged: customFieldsModel.setProperty(index, "value", text)
                }
                FlatButton { text: "-"; palette: root.palette; onClicked: customFieldsModel.remove(index) }
            }
        }

        FlatButton { text: "Add Field"; palette: root.palette; onClicked: customFieldsModel.append({ key: "", value: "" }) }

        Label { text: "Inline Help Entries"; color: palette.textPrimary; font.bold: true }

        ListModel { id: helpEntriesModel }

        Repeater {
            model: helpEntriesModel
            delegate: RowLayout {
                Layout.fillWidth: true
                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Term"
                    text: model.term
                    onTextChanged: helpEntriesModel.setProperty(index, "term", text)
                }
                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Explanation"
                    text: model.text
                    onTextChanged: helpEntriesModel.setProperty(index, "text", text)
                }
                FlatButton { text: "-"; palette: root.palette; onClicked: helpEntriesModel.remove(index) }
            }
        }

        FlatButton { text: "Add Help Entry"; palette: root.palette; onClicked: helpEntriesModel.append({ term: "", text: "" }) }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Label {
                text: root.saveError
                color: palette.danger
                visible: text.length > 0
            }
            FlatButton { text: "Cancel"; palette: root.palette; onClicked: root.cancelRequested() }
            FlatButton {
                text: "Save"
                palette: root.palette
                onClicked: {
                    const custom = {}
                    for (let i = 0; i < customFieldsModel.count; ++i) {
                        const item = customFieldsModel.get(i)
                        if (item.key.trim().length > 0) {
                            custom[item.key.trim()] = item.value
                        }
                    }

                    const help = {}
                    for (let j = 0; j < helpEntriesModel.count; ++j) {
                        const h = helpEntriesModel.get(j)
                        if (h.term.trim().length > 0) {
                            help[h.term.trim()] = h.text
                        }
                    }

                    const resources = []
                    for (let r = 0; r < resourcesModel.count; ++r) {
                        const res = resourcesModel.get(r)
                        resources.push({
                            name: (res.name || "").trim(),
                            current: Number(res.current),
                            max: Number(res.max)
                        })
                    }

                    root.saveRequested({
                        id: idField.text.trim(),
                        parentId: skill.parentId || "",
                        name: nameField.text,
                        iconPath: iconField.text,
                        shortDescription: shortField.text,
                        longDescription: longField.text,
                        isLocked: lockedCheck.checked,
                        isCollapsed: skill.isCollapsed || false,
                        resources: resources,
                        customFields: custom,
                        helpEntries: help
                    })
                }
            }
        }
    }

    FileDialog {
        id: iconDialog
        title: "Choose skill icon"
        nameFilters: ["Images (*.png *.jpg *.jpeg *.svg)", "All files (*)"]
        onAccepted: iconField.text = selectedFile
    }

    function reloadModels() {
        customFieldsModel.clear()
        const fields = skill.customFields || {}
        for (const k in fields) {
            customFieldsModel.append({ key: k, value: fields[k] })
        }

        helpEntriesModel.clear()
        const entries = helpEntriesMap || {}
        for (const term in entries) {
            helpEntriesModel.append({ term: term, text: entries[term] })
        }

        resourcesModel.clear()
        const resources = skill.resources || []
        for (let i = 0; i < resources.length; ++i) {
            const res = resources[i]
            resourcesModel.append({
                name: res.name || "Resource",
                current: Number(res.current ?? 0),
                max: Number(res.max ?? -1)
            })
        }
        if (resourcesModel.count === 0) {
            resourcesModel.append({ name: "Resource", current: 0, max: -1 })
        }
    }

    Component.onCompleted: reloadModels()
    onSkillChanged: reloadModels()
    onHelpEntriesMapChanged: reloadModels()
}
