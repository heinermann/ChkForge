#pragma once
#include <QObject>
#include <QButtonGroup>
#include <QAbstractButton>

#include <any>
#include <functional>

#include "DataMapper.h"

// TODO
namespace ChkForge {
  template<class T>
  class RadioDataMapper : public DataMapper<T> {
  public:
    RadioDataMapper(QTreeWidget* watchingTree, QButtonGroup* buttonGroup, T* watchingData, std::function<void()> updateFn)
      : DataMapper<T>(watchingTree, watchingData)
      , widget(buttonGroup)
      , update(updateFn)
    {
      this->connect(buttonGroup, &QButtonGroup::idClicked, this, &RadioDataMapper<T>::idClicked);
    }

    virtual ~RadioDataMapper() {}

    virtual T GetData(int id) const override {
      return this->data[id];
    }

    virtual void setUncertain() override {
      widget->setExclusive(false);
      for (QAbstractButton* btn : widget->buttons()) {
        btn->setChecked(false);
      }
      widget->setExclusive(true);
    }

    virtual void setValue(std::any value) override {
      QAbstractButton* btn = widget->button(std::any_cast<T>(value));
      if (btn != nullptr)
        btn->setChecked(true);
    }
  private:
    QButtonGroup* widget;
    std::function<void()> update;

    void idClicked(int idItem) {
      this->IterateSelectedIds([&](int id) {
        this->data[id] = static_cast<T>(idItem);
      });
      update();
    }
  };
}
