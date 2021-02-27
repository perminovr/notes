import QtQuick 2.11
import QtQuick.Controls 2.12
import QtQuick.Window 2.11
import QtQuick.Controls.Material 2.4

import Qt.WebSockets 1.1
import QtQuick.LocalStorage 2.12

import mypackage 1.0

ApplicationWindow {
    id : mainWindow

    width: 400
    height: 500
    visible: true
    //visibility: Window.FullScreen

    Material.theme: Material.Dark
    Material.accent: Material.Purple

    flags: Qt.Window | Qt.FramelessWindowHint

    property bool bImgChange: true

    RegItem {
        id: regItem
    }

    SecondWindowForm
    {
        butThird.onClicked: {
        }

        ledButton.onClicked: {
            ledImg.source = bImgChange? "redled.png" : "greenled.png"
            bImgChange = !bImgChange
        }

        exitButton.onClicked: {
            //mainWindow.close();
            uiBackEnd.changeValue(1,2);
        }

        butBackEnd.onClicked: {
            regItem.setItem(ledButton)
            regItem.setAddress(1)
            regItem.setProperty("text")
            uiBackEnd.regItem(regItem)
        }

        Component.onCompleted: {
            // AUTO REGISTER HERE
        }
    }
}
