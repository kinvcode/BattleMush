#include "taskeditdialog.h"
#include "ui_TaskEditDialog.h"

TaskEditDialog::TaskEditDialog(QWidget *parent, bool is_add)
    : QDialog(parent)
    , ui(new Ui::TaskEditDialog)
{
    ui->setupUi(this);
    m_is_add = is_add;
    initCompomentUI();
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
    // ui->spinId->setStyleSheet("");
    ui->spinId->setStyleSheet(
        "QSpinBox:disabled {"
        "   background-color: #f0f0f0;"
        "   color: #999999;"
        "   border: 1px solid #cccccc;"
        "}"
        );
    ui->timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

Task TaskEditDialog::getTask() const
{
    Task t;
    t.id = ui->spinId->value();

    QString type = ui->comboType->currentText();
    if (type == "刷图") t.type = TaskType::CleanRoom;
    else if (type == "休息") t.type = TaskType::Rest;
    else t.type = TaskType::Quest;

    t.option1 = ui->comboOption1->currentText();
    t.option2 = ui->comboOption2->currentText();
    t.runTime = ui->timeEdit->time();

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

    ui->comboOption1->setCurrentText(t.option1);
    ui->comboOption2->setCurrentText(t.option2);
    ui->timeEdit->setTime(t.runTime);
}

void TaskEditDialog::on_comboType_currentIndexChanged(int index)
{
    QString type = ui->comboType->currentText();

    ui->comboOption1->clear();
    ui->comboOption2->clear();

    if (type == "刷图")
    {
        ui->labelOption1->setText("地图");
        ui->labelOption2->setText("路线");

        ui->comboOption2->setVisible(true);
        ui->labelOption2->setVisible(true);

        // 模拟数据
        ui->comboOption1->addItems({"幽暗森林", "熔岩洞穴"});
        ui->comboOption2->addItems({"A->B->C", "快速路线"});
    }
    else if (type == "休息")
    {
        ui->labelOption1->setText("地图");
        ui->labelOption2->setText("休息点");

        ui->comboOption2->setVisible(true);
        ui->labelOption2->setVisible(true);

        ui->comboOption1->addItems({"主城", "温泉"});
        ui->comboOption2->addItems({"旅馆", "广场"});
    }
    else if (type == "任务")
    {
        ui->labelOption1->setText("任务名称");

        ui->comboOption2->setVisible(false);
        ui->labelOption2->setVisible(false);

        ui->comboOption1->addItems({"击败Boss", "收集材料"});
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

