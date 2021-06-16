#pragma once
#include <QObject>
#include <QCheckBox>

#include <any>

#include "DataMapper.h"

namespace ChkForge {
  template <typename T>
  class CheckboxDataMapper : public DataMapper<T> {
  public:
    CheckboxDataMapper(QTreeWidget* watchingTree, QCheckBox* checkboxWidget, T* watchingData, T flag)
      : DataMapper<T>(watchingTree, watchingData)
      , bitflag(flag)
      , widget(checkboxWidget)
    {
      this->connect(checkboxWidget, &QCheckBox::clicked, this, &CheckboxDataMapper<T>::clicked);
    }

    virtual ~CheckboxDataMapper() {}

    virtual T GetData(int id) const override {
      return this->data[id] & bitflag;
    }

    virtual void setUncertain() override {
      widget->setCheckState(Qt::PartiallyChecked);
    }

    virtual void setValue(std::any value) override {
      widget->setTristate(false);
      widget->setCheckState(std::any_cast<T>(value) ? Qt::Checked : Qt::Unchecked);
    }
  private:
    T bitflag;
    QCheckBox* widget;

    void clicked(bool checked) {
      widget->setTristate(false);

      Qt::CheckState state = widget->checkState();
      this->IterateSelectedIds([&](int id) {
        if (state == Qt::Checked)
          this->data[id] |= bitflag;
        else if (state == Qt::Unchecked)
          this->data[id] &= ~bitflag;
      });

    }
  };
}
