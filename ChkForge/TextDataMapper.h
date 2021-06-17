#pragma once

#include <QLineEdit>

#include "DataMapper.h"
#include "Utils.h"

namespace ChkForge {
  class TextDataMapper : public DataMapper<std::string> {
  public:
    TextDataMapper(QTreeWidget* watchingTree, QLineEdit* lineEditWidget, std::string* watchingData)
      : DataMapper<std::string>(watchingTree, watchingData)
      , widget(lineEditWidget)
    {
      this->connect(lineEditWidget, &QLineEdit::textEdited, this, &TextDataMapper::textEdited);
    }

    virtual ~TextDataMapper() {}

    virtual std::string GetData(int id) const override {
      return this->data[id];
    }

    virtual void setUncertain() override {
      widget->clear();
    }

    virtual void setValue(std::any value) override {
      QString text = QString::fromStdString(std::any_cast<std::string>(value));
      widget->setText(text);
    }
  private:
    QLineEdit* widget;

    void textEdited(const QString& text) {
      std::string stdText = toStdString(text);
      this->IterateSelectedIds([&](int id) {
        this->data[id] = stdText;
      });
    }
  };
}
