import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import ZSS 1.0 as ZSS
import Graph 1.0
Page{
    id:control;
    property bool socketConnect : false;
    property bool radioConnect : false;
    property bool medusaConnect : false;
    property bool simConnect : false;
    ZSS.Interaction{
        id:interaction;
    }
    header:TabBar {
        id: bar
        width:parent.width;
        contentHeight:50;
        height:contentHeight;
        position: TabBar.Header;
        TabButton {
            icon.source:"/source/camera.png";
        }
        TabButton {
            icon.source:"/source/referee.png";
        }
        TabButton {
            icon.source:"/source/joy3.png";
        }
        TabButton {
            icon.source:"/source/settings.png";
        }

    }
//    Timer {
//        id: ballspeedtimer
//        interval: 50
//        repeat: true
//        running: true
//        onTriggered: {
//            graph.removeFirstSample();
//            graph.appendSample(graph.newSample(++graph.offset));
//        }
//    }
    Timer{
        id:oneSecond;
        interval:1000;
        running:true;
        repeat:true;
        onTriggered: {
            radioComboBox.updateModel();
        }
    }

    StackLayout {
        id:controlLayout;
        width: parent.width;
        height:parent.height;
        currentIndex: bar.currentIndex;
        Grid {
            id:vision
            width:parent.width;
            height:parent.height;
            columnSpacing: 0;
            rowSpacing: 14;
            padding: 5;
            columns: 1;
            topPadding: 15;
            horizontalItemAlignment: Grid.AlignHCenter;
            verticalItemAlignment: Grid.AlignVCenter;
            anchors.horizontalCenter: parent.horizontalCenter;
            property int itemWidth : width - 2*padding;
            ZGroupBox{
                title: qsTr("Vision")
                Column{
                    id:visionControls;
                    width:parent.width;
                    height:parent.height;
                    spacing: 0;
                    padding:0;
                    property int itemWidth : width - 2*padding;
                    property bool ifConnected : false;
                    Grid{
                        id:cameraControls;
                        width:parent.itemWidth - 10;
                        columns:8;
                        columnSpacing: 0;
                        rowSpacing: 0;
                        horizontalItemAlignment: Grid.AlignHCenter;
                        verticalItemAlignment: Grid.AlignVCenter;
                        anchors.horizontalCenter: parent.horizontalCenter;
                        property int itemWidth : (width - (columns-1) * columnSpacing)/columns;
                        Repeater{
                            model:interaction.getCameraNumber();
                            CheckBox{
                                property int itemIndex : index;
                                property bool itemChecked : true;
                                checked: true
                                width:cameraControls.itemWidth;
                                height:40;
                                //anchors.fill: parent;
                                onCheckStateChanged: {
                                    interaction.controlCamera(itemIndex,checked);
                                }
                            }
                        }
                    }
                    Button{
                        width:parent.itemWidth;
                        icon.source:visionControls.ifConnected ? "/source/connect.png" : "/source/disconnect.png";
                        onClicked: {
                            visionControls.ifConnected = !visionControls.ifConnected;
                            interaction.setVision(visionControls.ifConnected);
                        }
                    }
                }
            }
/*
//                Graph {
//                    id: graph
//                    anchors.bottomMargin: 50
//                    anchors.verticalCenter: parent.verticalCenter;
//                    //非常奇怪，这里的区间是0-1，而且0在上方。
//                    function newSample(i) {
//                        return (-(interaction.getBallVelocity())/8000+1);//(Math.sin(i / 100.0 * Math.PI * 2) + 1) * 0.4 + Math.random() * 0.05;
//                    }

//                    Component.onCompleted: {
//                        for (var i=0; i<100; ++i)
//                            appendSample(newSample(i));
//                    }

//                    property int offset: 100;
//                    scale:1
//                }
*/
            ZGroupBox{
                title: qsTr("Radio")
                Grid{
                    width:parent.width;
                    verticalItemAlignment: Grid.AlignVCenter;
                    horizontalItemAlignment: Grid.AlignHCenter;
                    spacing: 0;
                    columns:1;
                    property int itemWidth : width - 2*padding;
                    SpinBox{
                        width:parent.itemWidth;
                        from:0;to:15;
                        wrap:true;
                        value:interaction.getFrequency();
                        onValueModified: {
                            if(!interaction.changeSerialFrequency(value))
                                value:interaction.getFrequency();
                        }
                    }
                    ComboBox{
                        id:radioComboBox;
                        enabled: !control.radioConnect;
                        model:interaction.getSerialPortsList();
                        onActivated: interaction.changeSerialPort(currentIndex);
                        width:parent.itemWidth;
                        function updateModel(){
                            model = interaction.getSerialPortsList();
                            if(currentIndex >= 0)
                                interaction.changeSerialPort(currentIndex);
                        }
                        Component.onCompleted: updateModel();
                    }
                    Button{
                        width:parent.itemWidth;
                        icon.source:control.radioConnect ? "/source/connect.png" : "/source/disconnect.png";
                        onClicked: {
                            control.radioConnect = !control.radioConnect;
                            if(!interaction.connectSerialPort(control.radioConnect)){
                                control.radioConnect = !control.radioConnect;
                            }
                        }
                    }
                }
            }
            ZGroupBox{
                title: qsTr("Medusa")
                Grid{
                    width:parent.width;
                    verticalItemAlignment: Grid.AlignVCenter;
                    horizontalItemAlignment: Grid.AlignHCenter;
                    spacing: 0;
                    rowSpacing: 5;
                    columns:1;
                    property int itemWidth : width - 2*padding;
                    ZSwitch{
                        id:medusaColor;
                        width:parent.itemWidth;
                        leftText:qsTr("Blue");
                        rightText:qsTr("Yellow");
                    }
                    ZSwitch{
                        id:medusaSide;
                        width:parent.itemWidth;
                        leftText:qsTr("Left");
                        rightText:qsTr("Right");
                    }
                    ZSwitch{
                        id:medusaVersion;
                        width:parent.itemWidth;
                        leftText:qsTr("Release");
                        rightText:qsTr("Debug");
                    }
                    Button{
                        width:parent.itemWidth;
                        icon.source:control.medusaConnect ? "/source/stop.png" : "/source/start.png";
                        onClicked: {
                            control.medusaConnect = !control.medusaConnect;
                            interaction.controlMedusa(control.medusaConnect,medusaColor.checked,medusaSide.checked,medusaVersion.checked)
                        }
                    }
                }
            }
            ZGroupBox{
                title: qsTr("ZSimulator")
                Grid{
                    width:parent.width;
                    verticalItemAlignment: Grid.AlignVCenter;
                    horizontalItemAlignment: Grid.AlignHCenter;
                    spacing: 0;
                    rowSpacing: 5;
                    columns:1;
                    property int itemWidth : width - 2*padding;
                    Button{
                        width:parent.itemWidth;
                        icon.source:control.simConnect ? "/source/stop.png" : "/source/start.png";
                        onClicked: {
                            control.simConnect = !control.simConnect;
                            interaction.controlSim(control.simConnect);
                        }
                    }
                }
            }
        }
        Grid {
            id: refereebox;
            width: parent.width;
            padding:10;
            verticalItemAlignment: Grid.AlignVCenter;
            horizontalItemAlignment: Grid.AlignHCenter;
            spacing: 0;
            columns:1;
            property int itemWidth : width - 2*padding;
        }
        /*****************************************/
        /*                  UDP                  */
        /*****************************************/
        Grid {
            id: radio;
            width: parent.itemWidth;
            padding:10;
            verticalItemAlignment: Grid.AlignVCenter;
            horizontalItemAlignment: Grid.AlignHCenter;
            spacing: 5;
            columns:1;
            property int itemWidth : width - 2*padding;
            ComboBox{
                id:networkInterfacesComboBox;
                enabled: !control.socketConnect;
                model:interaction.getNetworkInterfaces();
                onActivated: interaction.changeNetworkInterface(currentIndex);
                width:parent.itemWidth;
                function updateModel(){
                    model = interaction.getNetworkInterfaces();
                    if(currentIndex >= 0)
                        interaction.changeNetworkInterface(currentIndex);
                }
                Component.onCompleted: updateModel();
            }
            Button{
                width:parent.itemWidth;
                text:control.socketConnect ? "Disconnect" : "Connect";
                onClicked: {
                    control.socketConnect = !control.socketConnect;
                    if(!interaction.changeConnection(control.socketConnect)){
                        control.socketConnect = !control.socketConnect;
                    }
                }
            }
            RowLayout {
                id: textRowLayout
                TextField {
                    id: sendText
                    Layout.fillWidth: true
                    focus: true
                }
                Button {
                    text: 'Send'
                    onClicked: {
                        interaction.send(sendText.text);
                    }
                }
            }
        }
        /*****************************************/
        Settings{
            anchors.fill: parent;
        }
    }
}
