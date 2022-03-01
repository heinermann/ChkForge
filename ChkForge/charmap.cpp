#include "charmap.h"
#include "ui_charmap.h"

#include "CharacterWidget.h"

#include <QChar>

CharMap::CharMap(QWidget* parent, const QFont& font) :
  QDialog(parent),
  font(font),
  ui(std::make_unique<Ui::CharMap>())
{
  ui->setupUi(this);

  charWidget = new CharacterWidget;
  ui->scrollArea->setWidget(charWidget);
  charWidget->adjustSize();
  //charWidget->setFont(font);

  initCategories();
}

void CharMap::initCategories()
{
  QStringList category_names = {
    tr("All"),
    tr("Mark/NonSpacing"),
    tr("Mark/SpacingCombining"),
    tr("Mark/Enclosing"),
    tr("Number/DecimalDigit"),
    tr("Number/Letter"),
    tr("Number/Other"),
    tr("Separator/Space"),
    tr("Separator/Line"),
    tr("Separator/Paragraph"),
    tr("Other/Control"),
    tr("Other/Format"),
    tr("Other/Surrogate"),
    tr("Other/PrivateUse"),
    tr("Other/NotAssigned"),
    tr("Letter/Uppercase"),
    tr("Letter/Lowercase"),
    tr("Letter/Titlecase"),
    tr("Letter/Modifier"),
    tr("Letter/Other"),
    tr("Punctuation/Connector"),
    tr("Punctuation/Dash"),
    tr("Punctuation/Open"),
    tr("Punctuation/Close"),
    tr("Punctuation/InitialQuote"),
    tr("Punctuation/FinalQuote"),
    tr("Punctuation/Other"),
    tr("Symbol/Math"),
    tr("Symbol/Currency"),
    tr("Symbol/Modifier"),
    tr("Symbol/Other"),
    tr("Extended")
  };

  for (int i = 0; i < category_names.size(); ++i) {
    if (categories.count(i - 1) == 0) continue;
    ui->cmb_categories->addItem(category_names[i], i - 1);
  }
  ui->cmb_categories->setCurrentIndex(0);
}

CharMap::~CharMap() {}

void CharMap::on_cmb_categories_currentIndexChanged(int index)
{
  int categoryIndex = ui->cmb_categories->itemData(index).toInt();

  // TODO
}
