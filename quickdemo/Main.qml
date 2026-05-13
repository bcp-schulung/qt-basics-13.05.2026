import QtQuick
import QtQuick.Controls
import quickdemo  // gives access to the Counter C++ class

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Counter Demo")

    // Instantiate the C++ Counter object directly in QML
    Counter {
        id: counter
        threshold: 5

        // Connect to a C++ signal from QML
        onThresholdReached: (value) => {
            statusLabel.text = "Threshold reached at " + value + "!"
            statusLabel.color = "red"
        }

        // Property binding: QML reacts automatically whenever count changes
        onCountChanged: (value) => {
            console.log(counter.summary())  // calls Q_INVOKABLE method
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 16

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Count: " + counter.count  // bound to Q_PROPERTY
            font.pixelSize: 32
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Threshold: " + counter.threshold
            font.pixelSize: 16
            color: "grey"
        }

        Row {
            spacing: 8

            Button {
                text: "−"
                onClicked: counter.decrement()  // calls Q_INVOKABLE
            }

            Button {
                text: "Reset"
                onClicked: counter.reset()
            }

            Button {
                text: "+"
                onClicked: counter.increment()
            }
        }

        // Direct property write from QML → C++
        Row {
            spacing: 8

            Button {
                text: "Set threshold to 3"
                onClicked: counter.threshold = 3
            }

            Button {
                text: "Set threshold to 10"
                onClicked: {
                    counter.threshold = 10
                    statusLabel.text = ""
                    statusLabel.color = "black"
                }
            }
        }

        Text {
            id: statusLabel
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 14
        }
    }
}
