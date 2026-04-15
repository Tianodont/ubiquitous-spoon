import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import Qt5Compat.GraphicalEffects
import "components"

ApplicationWindow {
    id: root
    width: 1420
    height: 860
    visible: true
    title: "Skill Tree"

    property var palette: skillTreeVM.themeManager.palette
    property bool editorPanelVisible: false
    property var editingSkill: ({})
    property int menuMinWidth: 220
    property int menuItemHeight: 34

    color: palette.windowBg

    Component {
        id: flatMenuItem
        MenuItem {
            id: menuItem
            implicitHeight: root.menuItemHeight
            implicitWidth: root.menuMinWidth
            width: ListView.view ? ListView.view.width : implicitWidth
            contentItem: Label {
                text: menuItem.text
                color: palette.textPrimary
                leftPadding: 12
                rightPadding: 12
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                radius: 6
                color: menuItem.highlighted ? palette.menuHoverBg : "transparent"
            }
        }
    }

    menuBar: MenuBar {
        background: Rectangle {
            color: palette.panelBg
            border.width: 1
            border.color: palette.inputBorder
        }

        Menu {
            title: "Settings"
            delegate: flatMenuItem
            implicitWidth: root.menuMinWidth
            width: root.menuMinWidth
            background: Rectangle {
                radius: 10
                color: palette.menuBg
                border.color: palette.inputBorder
                border.width: 1
                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 0
                    verticalOffset: palette.menuShadowOffsetY || 2
                    radius: 10
                    samples: 24
                    color: palette.menuShadow || "#30000000"
                    transparentBorder: true
                }
            }

            MenuItem {
                text: skillTreeVM.editorMode ? "Editor Mode: ON" : "Editor Mode: OFF"
                onTriggered: skillTreeVM.editorMode = !skillTreeVM.editorMode
            }

            MenuSeparator {}

            MenuItem {
                text: "Export Tree..."
                onTriggered: exportDialog.open()
            }
            MenuItem {
                text: "Import Tree..."
                onTriggered: importDialog.open()
            }

            MenuSeparator {}

            Menu {
                title: "Theme"
                delegate: flatMenuItem
                implicitWidth: root.menuMinWidth
                width: root.menuMinWidth
                background: Rectangle {
                    radius: 10
                    color: palette.menuBg
                    border.color: palette.inputBorder
                    border.width: 1
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0
                        verticalOffset: palette.menuShadowOffsetY || 2
                        radius: 10
                        samples: 24
                        color: palette.menuShadow || "#30000000"
                        transparentBorder: true
                    }
                }
                MenuItem { text: "Dark RPG"; onTriggered: skillTreeVM.themeManager.currentTheme = "dark" }
                MenuItem { text: "Light Minimal"; onTriggered: skillTreeVM.themeManager.currentTheme = "light" }
                MenuItem { text: "Cyber"; onTriggered: skillTreeVM.themeManager.currentTheme = "cyber" }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: palette.panelBg
            border.color: palette.inputBorder
            border.width: 1

            SkillTreeCanvas {
                id: treeCanvas
                anchors.fill: parent
                anchors.margins: 8
                nodes: skillTreeVM.skillNodes
                edges: skillTreeVM.skillEdges
                palette: root.palette
                helpProvider: function(term) { return skillTreeVM.helpText(term) }
                diceRoller: function(skillId, expr) { return skillTreeVM.rollDiceForSkill(skillId, expr) }

                onSkillSelected: function(skillId) {
                    skillTreeVM.selectSkill(skillId)
                    root.editorPanelVisible = false
                }

                onSkillContextMenuRequested: function(skillId, gx, gy) {
                    contextSkillId = skillId
                    if (skillTreeVM.selectedSkill.id !== contextSkillId) {
                        skillTreeVM.selectSkill(contextSkillId)
                    }
                    const pos = root.contentItem.mapFromGlobal(gx, gy)
                    contextMenu.popup(pos.x, pos.y)
                }

                onSkillCollapseToggled: function(skillId) {
                    skillTreeVM.toggleCollapse(skillId)
                }

                onHelpRequested: function(term) {
                    treeHelpPopupText.text = skillTreeVM.helpText(term)
                    if (treeHelpPopupText.text.length === 0) {
                        treeHelpPopupText.text = "No help entry for '" + term + "'"
                    }
                    treeHelpPopup.open()
                }
            }

            Rectangle {
                id: dicePanel
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 12
                width: 120
                height: 86
                radius: 8
                color: palette.panelAltBg
                border.color: palette.inputBorder
                border.width: 1

                Column {
                    anchors.centerIn: parent
                    spacing: 2
                    Label {
                        text: "Last Roll"
                        color: palette.textSecondary
                        font.pixelSize: 11
                    }
                    Label {
                        text: skillTreeVM.lastRollExpression.length > 0 ? String(skillTreeVM.lastRollTotal) : "-"
                        color: palette.textPrimary
                        font.pixelSize: 30
                        font.bold: true
                    }
                }

                HoverHandler {
                    id: rollHover
                    onHoveredChanged: {
                        if (hovered && skillTreeVM.lastRollExpression.length > 0) {
                            diceTooltip.open()
                        } else {
                            diceTooltip.close()
                        }
                    }
                }
            }

            Popup {
                id: diceTooltip
                x: dicePanel.x
                y: dicePanel.y - height - 8
                padding: 10
                modal: false
                closePolicy: Popup.NoAutoClose

                enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 90 } }
                exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 80 } }

                background: Rectangle {
                    radius: 8
                    color: palette.tooltipBg
                    border.width: 1
                    border.color: palette.inputBorder
                }

                Label {
                    text: skillTreeVM.lastRollBreakdown
                    color: palette.tooltipText
                }
            }
        }

        Rectangle {
            id: panelResizer
            Layout.preferredWidth: 8
            Layout.fillHeight: true
            radius: 4
            color: resizeMouseArea.pressed ? palette.accentSoft : "transparent"

            property real startGlobalX: 0
            property int startWidth: 0

            MouseArea {
                id: resizeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                onPressed: function(mouse) {
                    const p = panelResizer.mapToGlobal(mouse.x, mouse.y)
                    panelResizer.startGlobalX = p.x
                    panelResizer.startWidth = skillTreeVM.editorPanelWidth
                }
                onPositionChanged: function(mouse) {
                    if (!(mouse.buttons & Qt.LeftButton)) {
                        return
                    }
                    const p = panelResizer.mapToGlobal(mouse.x, mouse.y)
                    const dx = panelResizer.startGlobalX - p.x
                    skillTreeVM.editorPanelWidth = panelResizer.startWidth + dx
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: skillTreeVM.editorPanelWidth
            Layout.fillHeight: true
            radius: 10
            color: palette.panelBg
            border.color: palette.inputBorder
            border.width: 1

            StackLayout {
                anchors.fill: parent
                anchors.margins: 12
                currentIndex: root.editorPanelVisible ? 1 : 0

                SkillDetailView {
                    palette: root.palette
                    skill: skillTreeVM.selectedSkill
                    helpProvider: function(term) { return skillTreeVM.helpText(term) }
                    diceRoller: function(expr) { return skillTreeVM.rollDiceForSkill(skillTreeVM.selectedSkill.id, expr) }
                    resourceAdjuster: function(delta) { skillTreeVM.adjustResource(skillTreeVM.selectedSkill.id, delta) }
                    resourceResetter: function() { skillTreeVM.resetResource(skillTreeVM.selectedSkill.id) }
                    resourceCurrentSetter: function(value) { skillTreeVM.setResourceCurrent(skillTreeVM.selectedSkill.id, value) }
                    resourceMaxSetter: function(value) { skillTreeVM.setResourceMax(skillTreeVM.selectedSkill.id, value) }
                    resourceAdjusterAt: function(index, delta) { skillTreeVM.adjustResourceAt(skillTreeVM.selectedSkill.id, index, delta) }
                    resourceCurrentSetterAt: function(index, value) { skillTreeVM.setResourceAtCurrent(skillTreeVM.selectedSkill.id, index, value) }
                    resourceMaxSetterAt: function(index, value) { skillTreeVM.setResourceAtMax(skillTreeVM.selectedSkill.id, index, value) }
                }

                SkillEditorView {
                    palette: root.palette
                    skill: root.editingSkill
                    helpEntriesMap: skillTreeVM.helpEntries
                    saveError: skillTreeVM.saveError

                    onSaveRequested: function(payload) {
                        skillTreeVM.updateOrCreateSkill(payload)

                        const help = payload.helpEntries || {}
                        const existing = skillTreeVM.helpEntries
                        for (const key in existing) {
                            if (!(key in help)) {
                                skillTreeVM.removeHelpEntry(key)
                            }
                        }
                        for (const term in help) {
                            skillTreeVM.setHelpEntry(term, help[term])
                        }

                        root.editorPanelVisible = false
                    }

                    onCancelRequested: root.editorPanelVisible = false
                }
            }
        }
    }

    Label {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        text: skillTreeVM.operationMessage
        color: palette.textSecondary
        visible: text.length > 0
        font.pixelSize: 11
    }

    property string contextSkillId: ""

    Menu {
        id: contextMenu
        delegate: flatMenuItem
        implicitWidth: root.menuMinWidth
        width: root.menuMinWidth
        background: Rectangle {
            radius: 10
            color: palette.menuBg
            border.color: palette.inputBorder
            border.width: 1
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: palette.menuShadowOffsetY || 2
                radius: 10
                samples: 24
                color: palette.menuShadow || "#30000000"
                transparentBorder: true
            }
        }

        MenuItem {
            text: "Edit skill"
            enabled: skillTreeVM.editorMode
            onTriggered: {
                if (skillTreeVM.selectedSkill.id !== contextSkillId) {
                    skillTreeVM.selectSkill(contextSkillId)
                }
                root.editingSkill = skillTreeVM.selectedSkill
                root.editorPanelVisible = true
            }
        }

        MenuItem {
            text: "Delete skill"
            enabled: skillTreeVM.editorMode
            onTriggered: skillTreeVM.deleteSkill(contextSkillId)
        }

        MenuItem {
            text: "Add child skill"
            enabled: skillTreeVM.editorMode
            onTriggered: {
                root.editingSkill = skillTreeVM.emptySkillForParent(contextSkillId)
                root.editorPanelVisible = true
            }
        }

        MenuSeparator {}

        MenuItem {
            text: skillTreeVM.selectedSkill.isLocked ? "Unlock skill" : "Lock skill"
            onTriggered: skillTreeVM.toggleSkillLock(contextSkillId)
        }

        MenuItem {
            text: skillTreeVM.selectedSkill.isCollapsed ? "Expand branch" : "Collapse branch"
            onTriggered: skillTreeVM.toggleCollapse(contextSkillId)
        }
    }

    FileDialog {
        id: exportDialog
        title: "Export skill tree"
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON (*.json)"]
        onAccepted: skillTreeVM.exportTree(selectedFile)
    }

    FileDialog {
        id: importDialog
        title: "Import skill tree"
        fileMode: FileDialog.OpenFile
        options: FileDialog.DontUseNativeDialog
        currentFolder: "file://" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        selectedNameFilter.index: 1
        onAccepted: skillTreeVM.importTree(selectedFile)
    }

    Popup {
        id: treeHelpPopup
        x: Math.max(12, root.width * 0.12)
        y: Math.max(12, root.height * 0.2)
        width: Math.min(420, root.width - 56)
        modal: false
        padding: 12
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        enter: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 100 } }
        exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 90 } }

        background: Rectangle {
            radius: 8
            color: palette.tooltipBg
            border.width: 1
            border.color: palette.inputBorder
        }

        Label {
            id: treeHelpPopupText
            width: parent.width
            wrapMode: Text.Wrap
            color: palette.tooltipText
        }
    }
}
