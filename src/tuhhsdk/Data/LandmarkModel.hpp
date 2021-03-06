#pragma once

#include <set>
#include <vector>

#include "Framework/DataType.hpp"
#include "LineData.hpp"
#include "Tools/Math/Eigen.hpp"
#include "Tools/Math/Line.hpp"
#include "Tools/Time.hpp"


class LandmarkModel : public DataType<LandmarkModel>
{
public:
  /// the name of this DataType
  DataTypeName name = "LandmarkModel";
  /**
   * @brief Goal stores to posts at once
   */
  struct Goal : public Uni::To, public Uni::From
  {
    /**
     * @brief Goal constructs a Goal from two posts
     */
    Goal()
    {
      left = {0, 0};
      right = {0, 0};
		hasOrientation = false;
		orientation = 0.f;
    }

    /**
     * @brief Goal constructs a Goal from two posts
     * @param left the left post of the goal
     * @param right the right post of the goal
     */
    Goal(const Vector2f& left, const Vector2f& right, const bool hasOrientation, const float& orientation)
      : left(left)
      , right(right)
      , hasOrientation(hasOrientation)
      , orientation(orientation)
    {
    }
    /// relative position of the left post
    Vector2f left;
    /// relative position of the right post
    Vector2f right;
	  /// the bool signifies whether the orientation is valid, float is the
	  /// angle between the nao's x axis and a line from the center of the goal to the penalty spot
	  bool hasOrientation;
	  float orientation;

    /**
     * @see function in DataType
     */
    virtual void toValue(Uni::Value& value) const
    {
      value = Uni::Value(Uni::ValueType::OBJECT);
      value["left"] << left;
      value["right"] << right;
      value["hasOrientation"] << hasOrientation;
      value["orientation"] << orientation;
    }

    /**
     * @see function in DataType
     */
    virtual void fromValue(const Uni::Value& value)
    {
      value["left"] >> left;
      value["right"] >> right;
      value["hasOrientation"] >> hasOrientation;
      value["orientation"] >> orientation;
    }
  };

  struct CenterCircle : public Uni::To, public Uni::From
  {
    /**
     * @brief CenterCircle constructs an empty CenterCircle object
     */
    CenterCircle()
    {
      position = {0, 0};
      hasOrientation = false;
      orientation = 0.f;
      usedLineIds = {};
    }
    /**
     * @brief Goal constructs a CenterCircle
     * @param left the left post of the goal
     * @param right the right post of the goal
     */
    CenterCircle(const Vector2f& position, const bool hasOrientation, const float& orientation,
                 const std::vector<size_t>& usedLineIds)
      : position(position)
      , hasOrientation(hasOrientation)
      , orientation(orientation)
      , usedLineIds(usedLineIds)
    {
    }
    /// relative position of the center circle
    Vector2f position;
    /// the bool signifies whether the orientation is valid, float is the
    /// angle between the nao's x axis and the long line through the center of the circle
    bool hasOrientation;
    float orientation;
    /// all line Ids that were used to create this center circle
    /// these correspond to the indicies of the lines in LineData.lines
    std::vector<size_t> usedLineIds;
    /**
     * @see function in DataType
     */
    virtual void toValue(Uni::Value& value) const
    {
      value = Uni::Value(Uni::ValueType::OBJECT);
      value["position"] << position;
      value["hasOrientation"] << hasOrientation;
      value["orientation"] << orientation;
      value["usedLineIds"] << usedLineIds;
    }

    /**
     * @see function in DataType
     */
    virtual void fromValue(const Uni::Value& value)
    {
      value["position"] >> position;
      value["hasOrientation"] >> hasOrientation;
      value["orientation"] >> orientation;
      value["usedLineIds"] >> usedLineIds;
    }
  };

  struct PenaltyArea : public Uni::To, public Uni::From
  {
    PenaltyArea()
    {
      position = {0.f, 0.f};
      hasOrientation = false;
      orientation = 0.f;
      usedLineIds = {};
    }

    PenaltyArea(const Vector2f& position, const bool hasOrientation, const float orientation,
                const std::vector<size_t>& usedLineIds)
      : position(position)
      , hasOrientation(hasOrientation)
      , orientation(orientation)
      , usedLineIds(usedLineIds)
    {
    }

    /// the position of the penalty area, defined by the penalty spot position
    Vector2f position;
    /// whether the penalty area has an orientation
    bool hasOrientation;
    /// the value of the orientation in radians
    float orientation;
    /// contains the line used for orientation calculation
    std::vector<size_t> usedLineIds;

    /**
     * @see function in DataType
     */
    virtual void toValue(Uni::Value& value) const
    {
      value = Uni::Value(Uni::ValueType::OBJECT);
      value["position"] << position;
      value["hasOrientation"] << hasOrientation;
      value["orientation"] << orientation;
      value["usedLineIds"] << usedLineIds;
    }

    /**
     * @see function in DataType
     */
    virtual void fromValue(const Uni::Value& value)
    {
      value["position"] >> position;
      value["hasOrientation"] >> hasOrientation;
      value["orientation"] >> orientation;
      value["usedLineIds"] >> usedLineIds;
    }
  };

  struct Intersection : public Uni::To, public Uni::From
  {
    // TODO: comments!
    enum class Type
    {
      UNDEFINED,
      X,
      T,
      L
    };

    Intersection()
    {
      type = Type::UNDEFINED;
      onLine1 = false;
      onLine2 = false;
      position = {0, 0};
      hasOrientation = false;
      orientation = 0.f;
      usedLineIds = {};
    }

    Type type;
    bool onLine1;
    bool onLine2;
    Vector2f position;
    bool hasOrientation;
    float orientation;
    std::vector<size_t> usedLineIds;

    Intersection(const Type type, const bool onLine1,
                 const bool online2, const Vector2f& position,
                 const bool hasOrientation, const float orientation,
                 const std::vector<size_t>& usedLineIds)
      : type(type)
      , onLine1(onLine1)
      , onLine2(online2)
      , position(position)
      , hasOrientation(hasOrientation)
      , orientation(orientation)
      , usedLineIds(usedLineIds)
    {
    }
    /**
     * @see function in DataType
     */
    virtual void toValue(Uni::Value& value) const
    {
      value = Uni::Value(Uni::ValueType::OBJECT);
      value["position"] << position;
      value["type"] << static_cast<int>(type);
      value["hasOrientation"] << hasOrientation;
      value["orientation"] << orientation;
      value["usedLineIds"] << usedLineIds;
    }

    /**
     * @see function in DataType
     */
    virtual void fromValue(const Uni::Value& value)
    {
      value["position"] >> position;
      int typeInt = 0;
      value["type"] >> typeInt;
      type = static_cast<Type>(typeInt);
      value["hasOrientation"] >> hasOrientation;
      value["orientation"] >> orientation;
      value["usedLineIds"] >> usedLineIds;
    }
  };

  /// a vector of complete goals
  std::vector<Goal> goals;
  /// a vector of center circles
  std::vector<CenterCircle> centerCircles;
  /// a vector of penalty areas
  std::vector<PenaltyArea> penaltyAreas;
  // a vector of intersections
  std::vector<Intersection> intersections;
  /// all lines after filtering
  std::vector<Line<float>> filteredLines;
  /// all line infos after filtering
  std::vector<LineInfo> filteredLineInfos;
  /// the distance threshold that was used for filtering the lines
  float maxLineProjectionDistance = 0.f;
  /// the timestamp of the image in which the landmarks were seen
  TimePoint timestamp;
  /**
   * @brief reset clears all vectors
   */
  void reset()
  {
    goals.clear();
    centerCircles.clear();
    penaltyAreas.clear();
    intersections.clear();
    filteredLines.clear();
    filteredLineInfos.clear();
  }

  virtual void toValue(Uni::Value& value) const
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["goals"] << goals;
    value["centerCircles"] << centerCircles;
    value["penaltyAreas"] << penaltyAreas;
    value["intersections"] << intersections;
    value["filteredLines"] << filteredLines;
    value["filteredLineInfos"] << filteredLineInfos;
    value["timestamp"] << timestamp;
  }

  virtual void fromValue(const Uni::Value& value)
  {
    value["goals"] >> goals;
    value["centerCircles"] >> centerCircles;
    value["penaltyAreas"] >> penaltyAreas;
    value["intersections"] >> intersections;
    value["filteredLines"] >> filteredLines;
    value["filteredLineInfos"] >> filteredLineInfos;
    value["timestamp"] >> timestamp;
  }
};
