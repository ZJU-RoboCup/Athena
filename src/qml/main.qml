import QtQuick 2.7
import QtQuick.Window 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import Client.Component 1.0 as Client
Window {
    id:root;
    visible: true;
    width: 1260;
    height: 740;
    minimumHeight: height;
    minimumWidth: width;
    maximumHeight: height;
    maximumWidth: width;
    color:"#e8e8e8";
    //flags:Qt.FramelessWindowHint

    Client.Interaction{ id : interaction; }

    Row {
        spacing: 0;
        TabView{
            id:fields;
            width:1080;
            height:740;
            Repeater{
                model:["Origin","Ball-Transformed","Robot-ModelFixed","Final-Montaged","Immortals"];
                Tab{
                    anchors.fill: parent;
                    title:modelData;
                    Client.Field{ type:index+1; }
                }
            }
            style: TabViewStyle {
                frameOverlap: 0
                tabOverlap: 0
                tab: Rectangle {
                    color: styleData.selected ? "grey" : "lightgrey"
                    implicitWidth: fields.width/fields.count;
                    implicitHeight: 20
                    Text {
                        id: text
                        anchors.centerIn: parent
                        text: styleData.title
                        color: styleData.selected ? "black" : "grey"
                    }
                }
            }
        }
        Column{
            width:180;
            height:parent.height;
            spacing: 0;
            Grid{
                id:cameraControls;
                width:parent.width;
                columns:1;
                columnSpacing: 0;
                rowSpacing: 0;
                anchors.horizontalCenter: parent.horizontalCenter;
                horizontalItemAlignment: Grid.AlignHCenter;
                verticalItemAlignment: Grid.AlignVCenter;
                property int itemWidth : (width - (columns-1) * columnSpacing)/columns;
                Repeater{
                    model:interaction.getCameraNumber();
                    Rectangle{
                        property int itemIndex : index;
                        property bool itemChecked : true;
                        color: itemChecked ? "#2baade" : "#e58a5c";
                        width:cameraControls.itemWidth;
                        height:20;
                        MouseArea{
                            anchors.fill: parent;
                            onClicked: {
                                parent.itemChecked = !parent.itemChecked;
                                interaction.controlCamera(itemIndex,itemChecked);
                            }
                        }
                    }
                }
            }
            Rectangle{
                property bool ifConnected : false;
                width:parent.width;
                height:40;
                color:ifConnected ? "#2bc88f" : "#e55a5c";
                MouseArea{
                    anchors.fill: parent;
                    onClicked: {
                        parent.ifConnected = !parent.ifConnected;
                        interaction.setVision(parent.ifConnected);
                    }
                }
            }
            Grid{
                id:processControl
                width:parent.width;
                columns:1;
                columnSpacing: 0;
                rowSpacing: 0;
                anchors.horizontalCenter: parent.horizontalCenter;
                horizontalItemAlignment: Grid.AlignHCenter;
                verticalItemAlignment: Grid.AlignVCenter;
                property int itemWidth : (width - (columns-1) * columnSpacing)/columns;
                Repeater{
                    model:["Transform","ModelFix","Montage"];
                    Rectangle{
                        property int itemIndex : index;
                        property bool itemChecked : true;
                        color: itemChecked ? "#7b99ff" : "#e52a9c";
                        width:processControl.itemWidth;
                        height:20;
                        MouseArea{
                            anchors.fill: parent;
                            onClicked: {
                                parent.itemChecked = !parent.itemChecked;
                                interaction.controlProcess(itemIndex,itemChecked);
                            }
                        }
                    }
                }
            }
        }
    }
}
