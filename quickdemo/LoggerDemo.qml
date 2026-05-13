import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import quickdemo

// ─────────────────────────────────────────────────────────────────────────────
// LoggerDemo — type in a message, send it to the C++ Logger class.
// The Logger calls qDebug() and keeps a history list that QML binds to.
// ─────────────────────────────────────────────────────────────────────────────
Item {
    // The C++ Logger registered with QML_ELEMENT
    Logger { id: logger }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        // ── Title ─────────────────────────────────────────────────────────────
        Label {
            Layout.fillWidth: true
            text: "QML → C++ Logging"
            font.pixelSize: 18
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
            color: "grey"
            text: "Text typed here is passed to Logger::log() in C++ via Q_INVOKABLE.\n" +
                  "Each call prints to qDebug() and appends to the history below."
        }

        // ── Input row ─────────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: inputField
                Layout.fillWidth: true
                placeholderText: "Type a message and press Log…"
                font.pixelSize: 14
                // Also submit on Enter
                Keys.onReturnPressed: sendMessage()
                Keys.onEnterPressed: sendMessage()
            }

            Button {
                text: "Log"
                enabled: inputField.text.trim().length > 0
                onClicked: sendMessage()
            }
        }

        // ── History ───────────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Log history (" + logger.history.length + " entries)"
                font.bold: true
            }

            Item { Layout.fillWidth: true }   // spacer

            Button {
                text: "Clear"
                enabled: logger.history.length > 0
                flat: true
                onClicked: logger.clearHistory()
            }
        }

        // Scrollable list of logged messages — bound to logger.history
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: historyView
                model: logger.history   // QStringList property — auto-updates
                spacing: 4

                // Scroll to the newest entry automatically
                onCountChanged: Qt.callLater(() => historyView.positionViewAtEnd())

                delegate: Rectangle {
                    width: historyView.width
                    height: entryLabel.implicitHeight + 10
                    color: index % 2 === 0 ? "#f0f0f0" : "#ffffff"
                    radius: 3

                    Label {
                        id: entryLabel
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 6 }
                        font.family: "Courier, monospace"
                        font.pixelSize: 12
                        // index + 1 so the first message is #1
                        text: "#" + (index + 1) + "  " + modelData
                        wrapMode: Text.WordWrap
                    }
                }

                // Placeholder shown when the list is empty
                Label {
                    anchors.centerIn: parent
                    visible: historyView.count === 0
                    text: "No messages logged yet."
                    color: "grey"
                    font.pixelSize: 13
                }
            }
        }
    }

    // ── Helper function ───────────────────────────────────────────────────────
    function sendMessage() {
        const msg = inputField.text.trim()
        if (msg.length === 0) return
        logger.log(msg)        // ← crosses the QML/C++ boundary
        inputField.text = ""
        inputField.forceActiveFocus()
    }
}
