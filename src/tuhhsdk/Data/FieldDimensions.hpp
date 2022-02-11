#pragma once

#include <cmath>

#include "Framework/DataType.hpp"
#include "Modules/Configuration/Configuration.h"
#include "Tools/Math/Eigen.hpp"

class FieldDimensions : public DataType<FieldDimensions>
{
public:

  enum FieldType {
    // field size of the official 2020 SPL
    FIELD_2020 = 0,
    // small robo ducks field
    FIELD_SMALL_ROBODUCKS = 1
  };

  /// the name of this DataType
  DataTypeName name = "FieldDimensions";
  /// the length of the field (A) [m]
  float fieldLength = 0.f;
  /// the width of the field (B) [m]
  float fieldWidth = 0.f;
  /// the width of the field lines (C) [m]
  float fieldLineWidth = 0.f;
  /// the size of the penalty marker (D) [m]
  float fieldPenaltyMarkerSize = 0.f;
  /// the length of the goalBox (E) [m]
  float fieldGoalBoxLength = 0.f;
  /// the width of the goal box (F) [m]
  float fieldGoalBoxWidth = 0.f;
  /// the length of the penalty area (G) [m]
  float fieldPenaltyAreaLength = 0.f;
  /// the width of the penalty area (H) [m]
  float fieldPenaltyAreaWidth = 0.f;
  /// the distance of the penalty marker to the end of the field (I) [m]
  float fieldPenaltyMarkerDistance = 0.f;
  /// the diameter of the center circle (J) [m]
  float fieldCenterCircleDiameter = 0.f;
  /// the width of the border strip (K) [m]
  float fieldBorderStripWidth = 0.f;
  /// the diameter of each goal post [m]
  float goalPostDiameter = 0.f;
  /// the height of each goal post [m]
  float goalHeight = 0.f;
  /// the distance between the inner points of the goal posts [m]
  float goalInnerWidth = 0.f;
  /// the depth of the goal [m]
  float goalDepth = 0.f;
  /// the diameter of the ball [m]
  float ballDiameter = 0.f;
  /// the length of the imaginary throw in line [m] (as of chapter 3.7 of the SPL rules 2018 (draft))
  float fieldThrowInLineLength = 0.f;
  /// the spacing of the imaginary throw in line from the side lines [m] (as of 3.7 of the SPL rules 2018 (draft))
  float fieldThrowInLineSpacing = 0.f;
  /// the type of the field
  FieldType fieldType;



  /**
   * @brief reset does nothing
   */
  void reset()
  {
  }
  /**
   * @brief isInsideField determines whether a ball is inside the field according to SPL rules
   * @param position a position in field coordinates (i.e. the center of the ball)
   * @param tolerance a tolerance value that the ball is allowed to be outside the field because of the uncertainty in the ball position
   * @return true iff the position is inside the field
   */
  bool isInsideField(const Vector2f& position, const float tolerance) const
  {
    return (std::abs(position.x()) < (fieldLength + ballDiameter + fieldLineWidth) * 0.5f + tolerance) &&
           (std::abs(position.y()) < (fieldWidth + ballDiameter + fieldLineWidth) * 0.5f + tolerance);
  }
  /**
   * @brief isInsideCarpet determines whether a position is on the carpet
   * @param position a position in field coordinates
   * @return true iff the position is on the carpet
   */
  bool isInsideCarpet(const Vector2f& position) const
  {
    return (std::abs(position.x()) < (fieldLength * 0.5f + fieldBorderStripWidth)) &&
           (std::abs(position.y()) < (fieldWidth * 0.5f + fieldBorderStripWidth));
  }

  virtual void toValue(Uni::Value& value) const
  {
    value = Uni::Value(Uni::ValueType::OBJECT);
    value["fieldLength"] << fieldLength;
    value["fieldWidth"] << fieldWidth;
    value["fieldLineWidth"] << fieldLineWidth;
    value["fieldPenaltyMarkerSize"] << fieldPenaltyMarkerSize;
    value["fieldGoalBoxLength"] << fieldGoalBoxLength;
    value["fieldGoalBoxWidth"] << fieldGoalBoxWidth;
    value["fieldPenaltyAreaLength"] << fieldPenaltyAreaLength;
    value["fieldPenaltyAreaWidth"] << fieldPenaltyAreaWidth;
    value["fieldPenaltyMarkerDistance"] << fieldPenaltyMarkerDistance;
    value["fieldCenterCircleDiameter"] << fieldCenterCircleDiameter;
    value["fieldBorderStripWidth"] << fieldBorderStripWidth;
    value["goalPostDiameter"] << goalPostDiameter;
    value["goalHeight"] << goalHeight;
    value["goalInnerWidth"] << goalInnerWidth;
    value["goalDepth"] << goalDepth;
    value["ballDiameter"] << ballDiameter;
    value["throwInLineLength"] << fieldThrowInLineLength;
    value["throwInLineSpacing"] << fieldThrowInLineSpacing;
  }

  virtual void fromValue(const Uni::Value& value)
  {
    getValueOrPanic(value, "fieldLength") >> fieldLength;
    getValueOrPanic(value, "fieldWidth") >> fieldWidth;
    getValueOrPanic(value, "fieldLineWidth") >> fieldLineWidth;
    getValueOrPanic(value, "fieldPenaltyMarkerSize") >> fieldPenaltyMarkerSize;
    getValueOrPanic(value, "fieldGoalBoxLength") >> fieldGoalBoxLength;
    getValueOrPanic(value, "fieldGoalBoxWidth") >> fieldGoalBoxWidth;
    getValueOrPanic(value, "fieldPenaltyAreaLength") >> fieldPenaltyAreaLength;
    getValueOrPanic(value, "fieldPenaltyAreaWidth") >> fieldPenaltyAreaWidth;
    getValueOrPanic(value, "fieldPenaltyMarkerDistance") >> fieldPenaltyMarkerDistance;
    getValueOrPanic(value, "fieldCenterCircleDiameter") >> fieldCenterCircleDiameter;
    getValueOrPanic(value, "fieldBorderStripWidth") >> fieldBorderStripWidth;
    getValueOrPanic(value, "goalPostDiameter") >> goalPostDiameter;
    getValueOrPanic(value, "goalHeight") >> goalHeight;
    getValueOrPanic(value, "goalInnerWidth") >> goalInnerWidth;
    getValueOrPanic(value, "goalDepth") >> goalDepth;
    getValueOrPanic(value, "ballDiameter") >> ballDiameter;
    getValueOrPanic(value, "throwInLineLength") >> fieldThrowInLineLength;
    getValueOrPanic(value, "throwInLineSpacing") >> fieldThrowInLineSpacing;
  }

  /**
   * @brief init loads the field dimensions from a configuration file
   * @param config a reference to the configuration provider
   */
  void init(Configuration& config)
  {
    config.mount("tuhhSDK.FieldType", "map.json", ConfigurationType::HEAD);

    auto fieldTypeConfig = config.get("tuhhSDK.FieldType");
    config.mount("tuhhSDK.FieldDimensions", fieldTypeConfig["fieldType"].asString() + ".json", ConfigurationType::HEAD);
    std::cout << "FieldJsonFile: " << fieldTypeConfig["fieldType"].asString() + ".json" << std::endl;

    // read field parameters
    auto group = config.get("tuhhSDK.FieldDimensions", "field");

    getValueOrPanic(group, "length") >> fieldLength;
    getValueOrPanic(group, "width") >> fieldWidth;
    getValueOrPanic(group, "lineWidth") >> fieldLineWidth;
    getValueOrPanic(group, "penaltyMarkerSize") >> fieldPenaltyMarkerSize;
    getValueOrPanic(group, "goalBoxLength") >> fieldGoalBoxLength;
    getValueOrPanic(group, "goalBoxWidth") >> fieldGoalBoxWidth;
    getValueOrPanic(group, "penaltyAreaLength") >> fieldPenaltyAreaLength;
    getValueOrPanic(group, "penaltyAreaWidth") >> fieldPenaltyAreaWidth;
    getValueOrPanic(group, "penaltyMarkerDistance") >> fieldPenaltyMarkerDistance;
    getValueOrPanic(group, "centerCircleDiameter") >> fieldCenterCircleDiameter;
    getValueOrPanic(group, "borderStripWidth") >> fieldBorderStripWidth;
    getValueOrPanic(group, "throwInLineLength") >> fieldThrowInLineLength;
    getValueOrPanic(group, "throwInLineSpacing") >> fieldThrowInLineSpacing;

    // read goal parameters
    group = config.get("tuhhSDK.FieldDimensions", "goal");

    getValueOrPanic(group, "postDiameter") >> goalPostDiameter;
    getValueOrPanic(group, "height") >> goalHeight;
    getValueOrPanic(group, "innerWidth") >> goalInnerWidth;
    getValueOrPanic(group, "depth") >> goalDepth;

    // read ball parameters
    group = config.get("tuhhSDK.FieldDimensions", "ball");
    getValueOrPanic(group, "diameter") >> ballDiameter;
  }

private:
  /**
   * @brief getValueOrPanic accesses a given json map with the given key. Throws exception if key does not exist.
   * @param group UniValue object containing a map.
   * @param key String containing the key to be accessed.
   * @return Uni::Value read from the map with the specified key.
   */
  Uni::Value getValueOrPanic(Uni::Value& group, const char* key){
    if (group.type() != Uni::ValueType::OBJECT)
    {
      throw std::runtime_error("Field Dimensions - Json Map Object given, is invalid!");
    }

    if (!group.hasProperty(key))
    {
      throw std::runtime_error("Field Dimensions - Key: '" + std::string(key) + "' is missing in json map.");
    }

    return group[key];
  }

  /**
   * @brief getValueOrPanic accesses a given json map with the given key. Throws exception if key does not exist.
   * @param group UniValue object containing a map.
   * @param key String containing the key to be accessed.
   * @return Uni::Value read from the map with the specified key.
   */
  const Uni::Value getValueOrPanic(const Uni::Value& group, const char* key){
    if (group.type() != Uni::ValueType::OBJECT)
    {
      throw std::runtime_error("Field Dimensions - Given json map object is invalid!");
    }

    if (!group.hasProperty(key))
    {
      throw std::runtime_error("Field Dimensions - Key: '" + std::string(key) + "' is missing in json map.");
    }

    return group[key];
  }
};


