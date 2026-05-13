import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import quickdemo

// ─────────────────────────────────────────────────────────────────────────────
// JsonDemo — demonstrates JSON serialisation and deserialisation two ways:
//   1. C++ side: JsonSerializer (QJsonDocument / QJsonObject / QJsonArray)
//   2. JS side:  JSON.stringify() / JSON.parse()  — built into the QML engine
// ─────────────────────────────────────────────────────────────────────────────
Item {
    id: root

    // The C++ helper registered with QML_ELEMENT
    JsonSerializer { id: serializer }

    // ── helpers ───────────────────────────────────────────────────────────────

    // Build a plain JS object from the input fields
    function buildPerson() {
        return {
            name:   nameField.text,
            age:    parseInt(ageField.text) || 0,
            city:   cityField.text,
            active: activeSwitch.checked
        }
    }

    // Build the sample contacts list used in the array section
    function buildContacts() {
        return [
            { name: "Alice",   city: "Berlin"    },
            { name: "Bob",     city: "London"    },
            { name: "Charlie", city: "New York"  }
        ]
    }

    // Display a parsed object in the result label
    function showParsed(obj) {
        if (!obj || Object.keys(obj).length === 0) {
            parsedLabel.color = "red"
            parsedLabel.text  = "⚠ Could not parse — is the JSON valid?"
            return
        }
        parsedLabel.color = Qt.darker("green", 1.2)
        parsedLabel.text  =
            "name:   " + obj.name   + "\n" +
            "age:    " + obj.age    + "\n" +
            "city:   " + obj.city   + "\n" +
            "active: " + obj.active
    }

    // ── scroll wrapper ────────────────────────────────────────────────────────
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 4

            // ── Title ─────────────────────────────────────────────────────────
            Label {
                Layout.fillWidth: true
                Layout.topMargin: 16
                text: "JSON Serialisation & Deserialisation"
                font.pixelSize: 18
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                Layout.bottomMargin: 8
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 12
                color: "grey"
                text: "C++ path uses QJsonDocument · JS path uses JSON.stringify / JSON.parse"
            }

            // ── Section A: Object serialisation ───────────────────────────────
            Label {
                Layout.leftMargin: 12
                text: "A — Object"
                font.bold: true
                font.pixelSize: 14
            }

            // Input
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                title: "1. Source object (edit the fields)"

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 8
                    rowSpacing: 4

                    Label { text: "name" }
                    TextField {
                        id: nameField
                        text: "Alice"
                        Layout.fillWidth: true
                    }

                    Label { text: "age" }
                    TextField {
                        id: ageField
                        text: "30"
                        Layout.fillWidth: true
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    Label { text: "city" }
                    TextField {
                        id: cityField
                        text: "Berlin"
                        Layout.fillWidth: true
                    }

                    Label { text: "active" }
                    Switch {
                        id: activeSwitch
                        checked: true
                    }
                }
            }

            // Serialise
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                title: "2. Serialise → JSON string"

                ColumnLayout {
                    width: parent.width
                    spacing: 6

                    RowLayout {
                        spacing: 8

                        Button {
                            text: "C++  (QJsonDocument)"
                            // Passes QVariantMap from QML → C++ → returns QString
                            onClicked: jsonArea.text = serializer.serialize(buildPerson())
                        }

                        Button {
                            text: "JS  (JSON.stringify)"
                            // Pure JavaScript — no C++ involved
                            onClicked: jsonArea.text = JSON.stringify(buildPerson(), null, 2)
                        }
                    }

                    Label { text: "Result (editable — modify and deserialise below):" }

                    TextArea {
                        id: jsonArea
                        Layout.fillWidth: true
                        font.family: "Courier, monospace"
                        font.pixelSize: 12
                        wrapMode: TextArea.Wrap
                        placeholderText: "Click a Serialise button above…"
                        background: Rectangle {
                            color: "#f5f5f5"
                            border.color: "#ccc"
                            radius: 4
                        }
                    }
                }
            }

            // Deserialise
            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                title: "3. Deserialise JSON string → object"

                ColumnLayout {
                    width: parent.width
                    spacing: 6

                    RowLayout {
                        spacing: 8

                        Button {
                            text: "C++  (QJsonDocument)"
                            onClicked: showParsed(serializer.deserialize(jsonArea.text))
                        }

                        Button {
                            text: "JS  (JSON.parse)"
                            onClicked: {
                                try {
                                    showParsed(JSON.parse(jsonArea.text))
                                } catch (e) {
                                    parsedLabel.color = "red"
                                    parsedLabel.text  = "Parse error: " + e.message
                                }
                            }
                        }
                    }

                    Label {
                        id: parsedLabel
                        Layout.fillWidth: true
                        font.family: "Courier, monospace"
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        text: "(no result yet)"
                        color: "grey"
                    }
                }
            }

            // ── Section B: Array serialisation ────────────────────────────────
            Label {
                Layout.leftMargin: 12
                Layout.topMargin: 8
                text: "B — Array"
                font.bold: true
                font.pixelSize: 14
            }

            GroupBox {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                Layout.bottomMargin: 12
                title: "Serialise / deserialise a list of objects"

                ColumnLayout {
                    width: parent.width
                    spacing: 6

                    RowLayout {
                        spacing: 8

                        Button {
                            text: "C++  serialize list"
                            onClicked: listArea.text = serializer.serializeList(buildContacts())
                        }

                        Button {
                            text: "JS  JSON.stringify"
                            onClicked: listArea.text = JSON.stringify(buildContacts(), null, 2)
                        }
                    }

                    TextArea {
                        id: listArea
                        Layout.fillWidth: true
                        font.family: "Courier, monospace"
                        font.pixelSize: 12
                        wrapMode: TextArea.Wrap
                        placeholderText: "Click a button above to serialise the contacts list…"
                        background: Rectangle {
                            color: "#f5f5f5"
                            border.color: "#ccc"
                            radius: 4
                        }
                    }

                    RowLayout {
                        spacing: 8

                        Button {
                            text: "C++  deserialize list"
                            onClicked: {
                                const items = serializer.deserializeList(listArea.text)
                                listResultLabel.color = items.length > 0 ? Qt.darker("green", 1.2) : "red"
                                listResultLabel.text  = items.length > 0
                                    ? items.map(function(p) {
                                        return p.name + " — " + p.city
                                    }).join("\n")
                                    : "⚠ Could not parse — is the JSON a valid array?"
                            }
                        }

                        Button {
                            text: "JS  JSON.parse"
                            onClicked: {
                                try {
                                    const items = JSON.parse(listArea.text)
                                    listResultLabel.color = Qt.darker("green", 1.2)
                                    listResultLabel.text  = items.map(function(p) {
                                        return p.name + " — " + p.city
                                    }).join("\n")
                                } catch (e) {
                                    listResultLabel.color = "red"
                                    listResultLabel.text  = "Parse error: " + e.message
                                }
                            }
                        }
                    }

                    Label {
                        id: listResultLabel
                        Layout.fillWidth: true
                        font.family: "Courier, monospace"
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        text: "(no result yet)"
                        color: "grey"
                    }
                }
            }
        }
    }
}
