import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import ZSS 1.0 as ZSS
Item{
    ZSS.ParamModel{
        id:paramModel;
    }
    anchors.fill: parent
    TableView{
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        TableViewColumn {
            title: "Name"
            role: "settingName"
            width:parent.width*0.5;
        }
        TableViewColumn {
            title: "Type"
            role: "settingType"
            width:parent.width*0.2;
        }
        TableViewColumn {
            title: "Value"
            role: "settingValue"
            width:parent.width*0.3;
            delegate:stringDelegate;
        }
        model:paramModel;
        style: TableViewStyle {
            backgroundColor: "#484848";
            alternateBackgroundColor:"#404040";
            textColor:"#ccc";
            headerDelegate: Rectangle {
                height: headerItem.implicitHeight*1.4
                width: headerItem.implicitWidth
                color: "#333"
                Text {
                    id: headerItem
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: styleData.textAlignment
                    anchors.leftMargin: 12
                    text: styleData.value
                    elide: Text.ElideRight
                    color: "#fff";
                    renderType: Text.NativeRendering
                    font.pixelSize: 16;
                    font.family: "Arial";
                }
                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 1
                    anchors.topMargin: 1
                    width: 1
                    color: "#777"
                }
                Rectangle {
                    anchors.right: parent.right
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 1
                    anchors.rightMargin: 1
                    height:1
                    color: "#777"
                }
            }
            rowDelegate:Rectangle{
                height: 30;
                property color selectedColor: control.activeFocus ? "#07c" : "#999"
                //color: styleData.selected ? selectedColor : !styleData.alternate ? alternateBackgroundColor : backgroundColor
                color: !styleData.alternate ? alternateBackgroundColor : backgroundColor
            }

            itemDelegate: normalTextDelegate;
        }
    }
    Component {
        id: stringDelegate
        Rectangle{
            color: input.activeFocus  ? "#ccc" : "transparent";
            height:30;
            TextInput {
                id:input;
                anchors.fill: parent;
                text: styleData.value
                color:activeFocus?"#222":styleData.textColor;
                horizontalAlignment: styleData.textAlignment
                font.pixelSize: 14;
                font.family:"Arial";
                anchors.leftMargin: 12
                verticalAlignment: Text.AlignVCenter
                //validator:RegExpValidator { regExp: /^(-?)(0|([1-9][0-9]*))(\.[0-9]+)?$/ }
                onAccepted:{
                    if(paramModel.setData(styleData.row,styleData.column,text))
                        focus = false;
                }
            }
        }
    }
    Component{
        id: normalTextDelegate
        Text {
            id: textItem
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: styleData.textAlignment
            anchors.leftMargin: 12
            text: styleData.value
            color: styleData.textColor;
            font.pixelSize: 14;
            font.family: "Arial";
        }
    }
}
