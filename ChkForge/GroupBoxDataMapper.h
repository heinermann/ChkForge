#pragma once

#include "DataMapper.h"
#include "tristategroupbox.h"

namespace ChkForge {
  template <typename T>
  class GroupBoxDataMapper : public DataMapper<T> {
  public:
    GroupBoxDataMapper(QTreeWidget* watchingTree, TriStateGroupBox* checkboxWidget, T* watchingData, T flag)
      : DataMapper<T>(watchingTree, watchingData)
      , bitflag(flag)
      , widget(checkboxWidget)
    {
      this->connect(checkboxWidget, &TriStateGroupBox::clicked, this, &GroupBoxDataMapper<T>::clicked);
    }

    virtual ~GroupBoxDataMapper() {}

    virtual T GetData(int id) const override {
      return this->data[id] & bitflag;
    }

    virtual void setUncertain() override {
      widget->setPartiallyChecked(true);
    }

    virtual void setValue(std::any value) override {
      widget->setPartiallyChecked(false);
      widget->setChecked(std::any_cast<T>(value) != 0);
    }
  private:
    T bitflag;
    TriStateGroupBox* widget;

    void clicked(bool) {
      bool checked = widget->isChecked();
      widget->setPartiallyChecked(false);

      this->IterateSelectedIds([&](int id) {
        if (checked)
          this->data[id] |= bitflag;
        else
          this->data[id] &= ~bitflag;
        });

    }
  };
}
