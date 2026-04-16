#include "taskeditdialog.h"
#include "ui_TaskEditDialog.h"
#include "global/controller.h"

TaskEditDialog::TaskEditDialog(QWidget *parent, bool is_add)
    : QDialog(parent)
    , ui(new Ui::TaskEditDialog)
{
    ui->setupUi(this);
    m_is_add = is_add;

    initCompomentUI();

    initData();

    // qDebug() << "初始化完毕！！";
}

TaskEditDialog::~TaskEditDialog()
{
    delete ui;
}

void TaskEditDialog::initCompomentUI()
{
    ui->comboType->addItem(QIcon(":/assets/images/icons/1.svg"), "刷图");
    ui->comboType->addItem(QIcon(":/assets/images/icons/2.svg"), "休息");
    ui->comboType->addItem(QIcon(":/assets/images/icons/4.svg"), "任务");

    ui->spinId->setDisabled(true);
    ui->spinId->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->spinId->setStyleSheet(
        "QSpinBox:disabled {"
        "   background-color: #f0f0f0;"
        "   color: #999999;"
        "   border: 1px solid #cccccc;"
        "}"
        );
    ui->timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void TaskEditDialog::initData()
{

}

void TaskEditDialog::flushCleanRoomOption()
{
    for(const auto &room_path : Gcontroller.m_clean_room_list){
        ui->comboOption1->addItem(room_path.name, room_path.id);
    }
}

void TaskEditDialog::flushRestOption()
{
    for(const auto &rest : Gcontroller.m_rest_point_list){
        ui->comboOption1->addItem(rest.name, rest.id);
    }
}

void TaskEditDialog::flushQuestOption()
{
    for(const auto &quest : Gcontroller.m_quest_list){
        ui->comboOption1->addItem(quest.name, quest.id);
    }
}

void TaskEditDialog::flushCleanRoomPath(int room_id)
{
    // qDebug() << "当前选择的刷图地图ID：" << room_id;
    ui->comboOption2->clear();
    for(const auto &room_path : Gcontroller.m_clean_room_list){
        if(room_path.id != room_id) continue;
        for(const auto &path : room_path.paths){
            ui->comboOption2->addItem(path.name, path.id);
        }
    }
}

void TaskEditDialog::flushRestPoint(int rest_id)
{
    // qDebug() << "当前选择的休息地图ID：" << rest_id;
    ui->comboOption2->clear();
    for(const auto &room_rest : Gcontroller.m_rest_point_list){
        if(room_rest.id != rest_id) continue;
        for(const auto &rest : room_rest.rests){
            ui->comboOption2->addItem(rest.name, rest.id);
        }
    }
}

Task TaskEditDialog::getTask() const
{
    Task t;
    t.id = ui->spinId->value();

    int type = ui->comboType->currentIndex();
    if (type == 0) t.type = TaskType::CleanRoom;
    else if (type == 1) t.type = TaskType::Rest;
    else if(type == 2) t.type = TaskType::Quest;

    t.option1_key = ui->comboOption1->currentData().toInt();
    t.option1_value = ui->comboOption1->currentText();
    t.option2_key = ui->comboOption2->currentData().toInt();
    t.option2_value = ui->comboOption2->currentText();
    QTime time = ui->timeEdit->time();
    t.runTime = (time.hour() * 3600) + (time.minute() * 60) + time.second();

    qDebug() << "任务序号：" << t.id;
    qDebug() << "任务类型：" << type;
    qDebug() << "任务选项1索引：" << t.option1_key;
    qDebug() << "任务选项1值：" << t.option1_value;
    qDebug() << "任务选项2索引：" << t.option2_key;
    qDebug() << "任务选项2值：" << t.option2_value;
    qDebug() << "任务时间：" << t.runTime;

    return t;
}

void TaskEditDialog::setTask(const Task& t)
{
    ui->spinId->setValue(t.id);

    switch (t.type)
    {
    case TaskType::CleanRoom: ui->comboType->setCurrentText("刷图"); break;
    case TaskType::Rest:  ui->comboType->setCurrentText("休息"); break;
    case TaskType::Quest:  ui->comboType->setCurrentText("任务"); break;
    }

    on_comboType_currentIndexChanged(0); // 刷新选项

    ui->comboOption1->setCurrentText(t.option1_value);
    ui->comboOption2->setCurrentText(t.option2_value);
    QTime time = QTime::fromMSecsSinceStartOfDay(t.runTime * 1000);
    ui->timeEdit->setTime(time);
}

void TaskEditDialog::on_comboType_currentIndexChanged(int index)
{
    int type = ui->comboType->currentIndex();

    ui->comboOption1->clear();
    ui->comboOption2->clear();

    if (type == 0)
    {
        ui->labelOption1->setText("地图");
        ui->labelOption2->setText("路线");

        ui->comboOption2->setVisible(true);
        ui->labelOption2->setVisible(true);

        flushCleanRoomOption();
    }
    else if (type == 1)
    {
        ui->labelOption1->setText("地图");
        ui->labelOption2->setText("休息点");

        ui->comboOption2->setVisible(true);
        ui->labelOption2->setVisible(true);

        flushRestOption();
    }
    else if (type == 2)
    {
        ui->labelOption1->setText("任务名称");

        ui->comboOption2->setVisible(false);
        ui->labelOption2->setVisible(false);

        flushQuestOption();
    }
}


void TaskEditDialog::on_pushButton_save_clicked()
{
    accept();
}


void TaskEditDialog::on_pushButton_cancel_clicked()
{
    reject();
}


void TaskEditDialog::on_comboOption1_currentIndexChanged(int index)
{
    int type = ui->comboType->currentIndex();

    int option1 = ui->comboOption1->currentIndex();

    if( type == 0 ){
        flushCleanRoomPath(option1);
    }
    else if (type == 1){
        flushRestPoint(option1);
    }

}

