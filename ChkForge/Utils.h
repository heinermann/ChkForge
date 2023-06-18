#pragma once

#include <QString>
#include <QPoint>
#include <QRect>

#include <string>
#include <utility>
#include <tuple>
#include <algorithm>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include "../openbw/openbw/util.h"

#include <MappingCoreLib/Chk.h>

std::string toStdString(const QString& str);

namespace ChkForge {
  using BoostPt = boost::geometry::model::point<int, 2, boost::geometry::cs::cartesian>;
  using BoostRect = boost::geometry::model::box<BoostPt>;

  struct rect;

  struct pt : std::pair<int, int> {
    using backed_t = std::pair<int, int>;
    using backed_t::pair;

    pt(const BoostPt &other) : backed_t{ other.get<0>(), other.get<1>() } {}
    pt(const QPoint &other) : backed_t{ other.x(), other.y() } {}
    pt(const bwgame::xy &other) : backed_t{ other.x, other.y } {}

    constexpr int x() const { return first; }
    constexpr int y() const { return second; }

    operator BoostPt() const { return { x(), y() }; }
    operator QPoint() const { return { x(), y() }; }
    operator bwgame::xy() const { return { x(), y() }; }

    pt& operator *=(int v) { first *= v; second *= v; return *this; }
    pt& operator /=(int v) { first /= v; second /= v; return *this; }
    pt& operator += (const pt& other) { first += other.first; second += other.second; return *this; }
    pt& operator -= (const pt& other) { first -= other.first; second -= other.second; return *this; }

    pt operator *(int v) const { return pt(*this) *= v; }
    pt operator /(int v) const { return pt(*this) /= v; }
    pt operator +(const pt& other) const { return pt(*this) += other; }
    pt operator -(const pt& other) const { return pt(*this) -= other; }

    pt clamped(rect rct) const;
  };

  struct rect : std::tuple<int, int, int, int> {
    using backed_t = std::tuple<int, int, int, int>;
    using backed_t::tuple;

    rect(const BoostRect& other) : backed_t{ other.min_corner().get<0>(), other.min_corner().get<1>(), other.max_corner().get<0>(), other.max_corner().get<1>() } {}
    rect(const QRect& other) : backed_t{ other.left(), other.top(), other.right(), other.bottom() } {}
    rect(const bwgame::rect& other) : backed_t{ other.from.x, other.from.y, other.to.x, other.to.y } {}
    rect(const Chk::Location& other) : backed_t{ other.left, other.top, other.right, other.bottom } {}
    rect(const Chk::LocationPtr& other) : rect{ *other } {}

    constexpr int left() const { return std::get<0>(*this); }
    constexpr int top() const { return std::get<1>(*this); }
    constexpr int right() const { return std::get<2>(*this); }
    constexpr int bottom() const { return std::get<3>(*this); }

    pt topLeft() const { return pt{ left(), top() }; }
    pt bottomRight() const { return pt{ right(), bottom() }; }

    operator BoostRect() const { return { topLeft(), bottomRight() }; }
    operator QRect() const { return { topLeft(), bottomRight() }; }
    operator bwgame::rect() const { return { topLeft(), bottomRight() }; }
  };
}
