#include "scenariodescription.h"
#include "ui_scenariodescription.h"

#include <QDialog>
#include <QAction>
#include <QMessageBox>

#include "charmap.h"

ScenarioDescription::ScenarioDescription(QWidget* parent) :
  QDialog(parent),
  ui(std::make_unique<Ui::ScenarioDescription>())
{
  ui->setupUi(this);
}

ScenarioDescription::~ScenarioDescription() {}

int ScenarioDescription::exec() {
  QFont namefont{};
  namefont.setFamilies(QStringList{ "EurostileExtReg", "BlizzardGlobal", "Malgun Gothic", "UDTypos510B-P", "DejaVu Serif", "MS Gothic", "Segoe UI Symbol" });
  namefont.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
  namefont.setStyleHint(QFont::SansSerif, QFont::StyleStrategy(QFont::ForceOutline));
  ui->txt_name->setFont(namefont);

  QFont descfont{};
  descfont.setFamilies(QStringList{ "Eurostile", "BlizzardGlobal", "Malgun Gothic", "UDTypos58B-P", "DejaVu Serif", "MS Gothic", "Segoe UI Symbol" });
  descfont.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
  descfont.setStyleHint(QFont::SansSerif, QFont::StyleStrategy(QFont::ForceOutline));
  ui->txt_description->setFont(descfont);

  ui->txt_name->setPlainText(name);
  ui->txt_name->setAlignment(Qt::AlignCenter);
  ui->txt_description->setPlainText(description);

  return QDialog::exec();
}

void ScenarioDescription::accept() {
  name = ui->txt_name->toPlainText();
  description = ui->txt_description->toPlainText();
  
  name.replace("\n", "\r");
  description.replace("\n", "\r");

  QDialog::accept();
}

void ScenarioDescription::on_toolButton_pressed() {
  //auto charmap = CharMap(this, ui->txt_name->font());
  QFont fnt{ "EurostileExtReg" };
  auto charmap = CharMap(this, fnt);
  charmap.exec();
}