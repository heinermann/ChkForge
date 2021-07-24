#pragma once
#include <QObject>
#include <QTreeWidget>
#include <QMessageBox>

#include <any>
#include <set>

namespace ChkForge {
  template <class T>
  class DataMapper : public QObject {
  public:
    DataMapper(QTreeWidget* watchingTree, T* watchingData)
      : tree(watchingTree)
      , data(watchingData)
    {
      connect(tree, &QTreeWidget::itemSelectionChanged, this, &DataMapper<T>::OnSelectionChanged);
    }

    virtual ~DataMapper() {}

    template <typename Fn>
    void IterateSelectedIds(const Fn& fn) {
      for (QTreeWidgetItem* itm : tree->selectedItems()) {
        int id = itm->data(0, Qt::UserRole).toInt();
        fn(id);
      }
    }

    void OnSelectionChanged() {
      if (tree->selectedItems().empty()) return;

      uniques.clear();
      IterateSelectedIds([&](int id) { uniques.insert(GetData(id)); });

      if (uniques.size() > 1)
        setUncertain();
      else
        setValue(std::make_any<T>(*uniques.begin()));
    }

    virtual T GetData(int id) const {
      return data[id];
    }
    virtual void setUncertain() = 0;
    virtual void setValue(std::any value) = 0;

  protected:
    QTreeWidget* tree;
    T* data;

  private:
    std::set<T> uniques;
  };

}
