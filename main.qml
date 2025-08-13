import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3  // 需要 Qt Quick Dialogs 模块支持
import TaoQuick 1.0
ApplicationWindow {
    id: window
    width: 1420
    height: 800
    visible: true
    title: "Optimus 升级测试工具"

    Rectangle {
        anchors.fill: parent
        color: "#F5F7FA"
        RowLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: 16
            spacing: 24
            // ===== 左列 (设备信息 + 文件与升级 + 系统功能) 约30%宽度 =====
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: window.width * 0.3
                spacing: 12

                GroupBox {
                    title: "设备信息"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        TextField {
                            id: serialIdField
                            placeholderText: "序列ID"
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: deviceTypeCombo
                            model: ["500HZ", "1KHZ"]
                            Layout.fillWidth: true
                            onActivated: 
                            {
                            messageDialog.text = "你选择了: " + deviceTypeCombo.currentText
                            messageDialog.open()
                             }
                        }
   MessageDialog {
        id: messageDialog
        title: "提示"
        icon: StandardIcon.Information
        onAccepted: {
          controller.connectDevice()
        }
   }
                        ComboBox {
                            model: ["30秒", "60秒"]
                            Layout.fillWidth: true
                        }
                    }
                }

                GroupBox {
                    title: "文件与升级"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        TextField {
                            placeholderText: "客户FPGA传输"
                            Layout.fillWidth: true
                        }
                        CusButton_Red {
                            text: "选择ICU-STM32升级文件"
                            Layout.fillWidth: true
                        }
                        CusButton_Gradient {
                            text: "选择FPGA升级文件"
                            Layout.fillWidth: true
                        }
                        CusButton_Blue {
                            text: "升级ICU-BLE固件"
                            Layout.fillWidth: true
                        }
                    }
                }

                GroupBox {
                    title: "系统功能"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        Button { text: "恢复出厂设置"; Layout.fillWidth: true }
                        Button { text: "时间同步"; Layout.fillWidth: true }
                        Button { text: "传感器限制写入"; Layout.fillWidth: true }
                        Button { text: "电量计修正"; Layout.fillWidth: true }
                        Button { text: "电池健康度"; Layout.fillWidth: true }
                        Button { text: "信号质量"; Layout.fillWidth: true }
                        Button { text: "探头电流"; Layout.fillWidth: true }
                        Button { text: "ICU电量"; Layout.fillWidth: true }
                        Button { text: "FPGA版本"; Layout.fillWidth: true }
                        Button { text: "获取设备信息"; Layout.fillWidth: true }
                        Button { text: "读取下位机RTC时间"; Layout.fillWidth: true }
                    }
                }
            }

            // ===== 中列 (限制设置 + 设备数据) 约35%宽度 =====
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: window.width * 0.35
                spacing: 12

                GroupBox {
                    title: "限制设置"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        TextField {
                            placeholderText: "充电电流限制 100-250mA"
                            Layout.fillWidth: true
                        }
                        TextField {
                            placeholderText: "探头电流限制 16-50mA"
                            Layout.fillWidth: true
                        }
                        TextField {
                            placeholderText: "ICU温度限制 37.4°C"
                            Layout.fillWidth: true
                        }
                        TextField {
                            placeholderText: "超声超时 0-60s"
                            Layout.fillWidth: true
                        }
                        TextField {
                            placeholderText: "超温范围 39-43°C"
                            Layout.fillWidth: true
                        }
                        TextField {
                            placeholderText: "传感器采样时间 >=1"
                            Layout.fillWidth: true
                        }
                    }
                }

                GroupBox {
                    title: "设备数据"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        Button { text: "蓝牙配对"; Layout.fillWidth: true;onClicked: controller.connectDevice() }
                        Button { text: "ICU温度"; Layout.fillWidth: true }
                        Button { text: "电池充放电电流"; Layout.fillWidth: true }
                        Button { text: "ICU电池电压"; Layout.fillWidth: true }
                        Button { text: "ICU-STM32版本"; Layout.fillWidth: true }
                        Button { text: "BLE版本"; Layout.fillWidth: true }
                        Button { text: "设置设备信息"; Layout.fillWidth: true }
                    }

Connections {
        target: controller
        onBlePairedIdChanged: {
            serialIdField.text = id
        }
    }
                }
            }

            // ===== 右列 (采集与传感器 + 日志管理) 约35%宽度 =====
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: window.width * 0.35
                spacing: 12

                GroupBox {
                    title: "采集与传感器"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        Button { text: "传感器数据获取"; Layout.fillWidth: true }
                        Button { text: "写入"; Layout.fillWidth: true }
                        Button { text: "读取"; Layout.fillWidth: true }
                        Button { text: "升级"; Layout.fillWidth: true }
                        Button { text: "开始采集"; Layout.fillWidth: true }
                        Button { text: "结束采集"; Layout.fillWidth: true }
                    }
                }

                GroupBox {
                    title: "日志管理"
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 8
                        Button { text: "获取普通日志"; Layout.fillWidth: true }
                        Button { text: "获取紧急日志"; Layout.fillWidth: true }
                        Button { text: "获取日志占比"; Layout.fillWidth: true }
                        Button { text: "清除日志"; Layout.fillWidth: true }
                    }

              
                }
            }
        }
    }

    Component.onCompleted: {
        function applyStyle(item) {
            if (item instanceof TextField) {
                item.background = Qt.createQmlObject(`
                    import QtQuick 2.12
                    Rectangle {
                        color: "white"
                        border.color: "#bdc3c7"
                        radius: 4
                        border.width: 1
                    }`, item)
                item.implicitHeight = 32
            }
            if (item.children && item.children.length > 0) {
                for (var i = 0; i < item.children.length; i++) {
                    applyStyle(item.children[i])
                }
            }
        }
        applyStyle(mainLayout)
    }
}
