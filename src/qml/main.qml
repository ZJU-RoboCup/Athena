import QtQuick 2.7
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtQml 2.2
import ZSS 1.0 as ZSS
import Graph 1.0
Window {
    id:root;
    visible: true;
    width: screen.width - 20;
    height: screen.height - 100;
    minimumHeight: height;
    minimumWidth: width;
    maximumHeight: height;
    maximumWidth: width;
    color:"#e8e8e8";
//    flags:Qt.FramelessWindowHint
//    Shortcut{
//        sequence: "Ctrl+q"
//        onActivated: {
//            Qt.quit();
//        }
//    }

    Timer{
        id:fpsTimer;
        interval:1000;
        running:true;
        repeat:true;
        onTriggered: {
            fpsWord.fpsNum = (interaction.getFPS()).toString();
        }
    }
    ZSS.Interaction4Field { id : interaction }
    property bool ctrlC : false;
    Shortcut{
        sequence: "Ctrl+c";
        onActivated: {
            interaction.setCtrlC();
            ctrlC = !ctrlC;
        }
    }

    Row {
        spacing: 0;

        TabView{
            id:fields;
            width:(root.height-20)*4/3;
            height:root.height;
            currentIndex:1;
            onWidthChanged: {
                interaction.setSize(width,height - 20);
            }
            Repeater{
                model:["Origin","Filtered"];
                Tab{
                    anchors.fill: parent;
                    title:modelData;
                    Rectangle{
                        border.color: "#555";
                        border.width: 1;
                        color: "transparent";
                        anchors.fill: parent;
                        ZSS.Field{
                            type:index+1;
                            width:fields.width;
                            height:fields.height - 20;
                        }
                    }
                }
            }
            style: TabViewStyle {
                frameOverlap: 0
                tabOverlap: 0
                tab: Rectangle {
                    color: styleData.selected ? "#303030" : "grey"
                    implicitWidth: fields.width/fields.count;
                    implicitHeight: 20
                    Text {
                        id: text
                        anchors.centerIn: parent
                        text: styleData.title
                        color: styleData.selected ? "#dddddd" : "black"
                    }
                }
            }
            Text{
               property string fpsNum : "";
               id : fpsWord;
               text : qsTr("FPS   " + fpsNum);
               x:parent.width - 70;
               y:5;
               color:"white";
               font.pointSize: 10;
               font.weight:  Font.Bold;
            }
            Text{
                color:"white";x:10;y:5;
                property string strX : "0";
                property string strY : "0";
                id:positionDisplay;
                text:qsTr("( " + strX + " , " + strY + " )");
                font.pointSize: 10;
                font.weight:  Font.Bold;
            }
            Rectangle{
               id:areaRectangle;
               width:parent.width;
               height:parent.height;
               x:0;
               y:0;
               color:"#11ffffff";
               visible: false;
            }
            MouseArea{
                anchors.fill: parent;
                hoverEnabled: true;
                acceptedButtons: Qt.NoButton;
                onPositionChanged: {
                    positionDisplay.strX = (interaction.getRealX(mouseX)).toString();
                    positionDisplay.strY = (-interaction.getRealY(mouseY)).toString();
                    //console.log("hover : ",mouseX,mouseY);
                }
            }

            MouseArea{
                property int startX : 0;
                property int startY : 0;
                anchors.fill: parent;
                acceptedButtons: Qt.MiddleButton
                property bool controlMode : false
                onClicked: {
                    if(mouse.modifiers === Qt.AltModifier){
                        interaction.setPlacementPoint(mouseX,mouseY);
                    }
                }
                onPressed: {
                    controlMode = (mouse.modifiers === Qt.ControlModifier)
                    startX = mouseX;
                    startY = mouseY;
                    if(controlMode){
                        interaction.setArea(0,0,0,0);
                        areaRectangle.visible = true;
                        areaRectangle.width = 0;
                        areaRectangle.height = 0;
                        areaRectangle.x = startX;
                        areaRectangle.y = startY;
                    }
                }
                onPositionChanged: {
                    if(controlMode){
                        areaRectangle.x = Math.min(mouseX,startX);
                        areaRectangle.y = Math.min(mouseY,startY);
                        areaRectangle.width = Math.abs(mouseX - startX);
                        areaRectangle.height = Math.abs(mouseY - startY);
                    }else{
                        interaction.moveField(mouseX - startX,mouseY - startY)
                        startX = mouseX;
                        startY = mouseY;
                    }
                }
                onReleased: {
                    if(controlMode){
                        if(areaRectangle.width < 100 && areaRectangle.height < 100){
                            areaRectangle.x = areaRectangle.y = 0;
                            areaRectangle.width = areaRectangle.parent.width;
                            areaRectangle.height = areaRectangle.parent.height;
                            interaction.resetArea();
                        }else{
                            interaction.setArea(areaRectangle.x,areaRectangle.width + areaRectangle.x,areaRectangle.height + areaRectangle.y,areaRectangle.y);
                        }
                        areaRectangle.visible = false;
                    }
                    controlMode = false
                }
            }
        }
        ControlBoard{
            id:controlBoard;
            width:root.width - fields.width;
            height:parent.height;
        }
    }
}
