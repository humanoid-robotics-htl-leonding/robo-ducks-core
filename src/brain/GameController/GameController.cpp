#include <array>
#include <cstring>
#include <mutex>
#include <thread>

// This needs to be here because of windows includes
#include "Tools/Storage/Image.hpp"
#include "Tools/Storage/Image422.hpp"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Definitions/RoboCupGameControlData.h"

#include "print.h"

#include "GameController.hpp"


void GameController::registerForSocketReceive()
{
  socket_.async_receive_from(
      boost::asio::buffer(receive_), lastSenderEndpoint_,
      [this](const boost::system::error_code& error, std::size_t /* bytesTransferred */) {
        receivedFromNetwork_ = true;
        if (!error)
        {
          RoboCupGameControlData data;
          std::memcpy(&data, receive_.data(), sizeof(data));
          if (onControlDataReceived(data))
          {
            sendReturnDataMessage(GAMECONTROLLER_RETURN_MSG_ALIVE);
          }
        }
        else
        {
          print("Error receiving GameController message", LogLevel::ERROR);
        }
        registerForSocketReceive();
      });
}

void GameController::sendReturnDataMessage(uint8_t msg)
{
  // If no message arrived yet, the return address (lastSenderEndpoint_) is not initialized.
  if (!receivedFromNetwork_)
  {
    return;
  }

  RoboCupGameControlReturnData retDat;

  retDat.team = static_cast<uint8_t>(playerConfiguration_->teamNumber);
  retDat.player = static_cast<uint8_t>(playerConfiguration_->playerNumber);
  retDat.message = msg;

  std::memcpy(send_.data(), &retDat, send_.size() * sizeof(send_[0]));

  // send to the address from which the last packet was received
  gameControllerEndpoint_.address(lastSenderEndpoint_.address());
  print("Sending return data to GameController", LogLevel::DEBUG);
  socket_.async_send_to(
      boost::asio::buffer(send_), gameControllerEndpoint_,
      [](const boost::system::error_code& error, std::size_t /* bytesTransferred */) {
        if (error)
        {
          print("Failed sending return data to GameController", LogLevel::WARNING);
        }
        else
        {
          print("Successfully sent return data to GameController", LogLevel::DEBUG);
        }
      });
}

bool GameController::onControlDataReceived(const RoboCupGameControlData& data)
{
  print("Inside onControlDataReceived", LogLevel::DEBUG);

  // First do some sanity checks on the data.
  if (strncmp(data.header, GAMECONTROLLER_STRUCT_HEADER, sizeof(data.header)) != 0)
  {
    print("Message sanity check fails!", LogLevel::DEBUG);
    return false;
  }

  if (GAMECONTROLLER_STRUCT_VERSION != data.version)
  {
    print("GameControllerStructVersion missmatch!", LogLevel::ERROR);
    return false;
  }

  {
    std::lock_guard<std::mutex> lg(mutex_);
    if (data.teams[0].teamNumber == playerConfiguration_->teamNumber)
    {
      teamIndex_ = 0;
    }
    else if (data.teams[1].teamNumber == playerConfiguration_->teamNumber)
    {
      teamIndex_ = 1;
    }
    else
    {
      return false;
    }
    latestData_ = data;
    latestDataTimestamp_ = TimePoint::getCurrentTime();
    newNetworkData_ = true;
  }

  return true;
}

GameController::GameController(const ModuleManagerInterface& manager)
  : Module(manager)
  , forcePenaltyShootout_(*this, "forcePenaltyShootout")
  , playerConfiguration_(*this)
  , cycleInfo_(*this)
  , buttonData_(*this)
  , rawGameControllerState_(*this)
  , receive_()
  , send_()
  , ioService_()
  , socket_(ioService_)
  , gameControllerEndpoint_(boost::asio::ip::udp::v4(), GAMECONTROLLER_RETURN_PORT)
  , lastSenderEndpoint_()
  , receivedFromNetwork_(false)
  , latestData_()
  , teamIndex_(0)
  , newNetworkData_(false)
{
  internalState_.packetNumber = 0;
  internalState_.timestampOfLastMessage = latestDataTimestamp_;
  internalState_.playersPerTeam = 5;
  internalState_.type = CompetitionType::NORMAL;
  internalState_.competitionPhase = CompetitionPhase::ROUNDROBIN;
  internalState_.gameState = GameState::INITIAL;
  internalState_.gameStateChanged = latestDataTimestamp_;
  internalState_.gamePhase = GamePhase::NORMAL;
  internalState_.setPlay = SetPlay::NONE;
  internalState_.setPlayChanged = latestDataTimestamp_;
  internalState_.firstHalf = true;
  internalState_.kickingTeam = true;
  internalState_.kickingTeamNumber = static_cast<uint8_t>(playerConfiguration_->teamNumber);
  internalState_.secondaryTime = 0.f;
//  internalState_.dropInTeam = 0;
//  internalState_.dropInTime = -1.f;
  internalState_.remainingTime = 600.f;
  internalState_.teamColor = TeamColor::BLUE;
  internalState_.score = 0;
  internalState_.penalty = Penalty::NONE;
  internalState_.penalties.resize(MAX_NUM_PLAYERS, Penalty::NONE);
  internalState_.remainingPenaltyTime = 0.0f;
  internalState_.chestButtonWasPressedInInitial = false;

  receive_.fill(0);
  send_.fill(0);

  boost::asio::ip::udp::endpoint localEndpoint(boost::asio::ip::udp::v4(),
                                               GAMECONTROLLER_DATA_PORT);
  socket_.open(localEndpoint.protocol());
  socket_.set_option(boost::asio::socket_base::reuse_address(true));
  socket_.bind(localEndpoint);

  registerForSocketReceive();

  backgroundThread_ = std::make_shared<std::thread>([this]() { ioService_.run(); });
}

GameController::~GameController()
{
  ioService_.stop();
  backgroundThread_->join();
  socket_.close();
}

void GameController::cycle()
{
  handleNetwork();
  handleChestButton();
  internalState_.valid = true;
  *rawGameControllerState_ = internalState_;
  // hack alert: This is for the file transport so that it knows whether it should record or write
  // data.
  debug().update("GameController.penalizedOrFinished",
                 internalState_.penalty != Penalty::NONE ||
                     internalState_.gameState == GameState::FINISHED);
}

void GameController::handleNetwork()
{
  // Do not incorporate network updates as long as the chest button has not been pressed in initial.
  if (internalState_.gameState == GameState::INITIAL && !internalState_.chestButtonWasPressedInInitial)
  {
    return;
  }
  std::lock_guard<std::mutex> lg(mutex_);
  if (!newNetworkData_)
  {
    return;
  }
  newNetworkData_ = false;
  // try-catch because the conversion functions may throw.
  try
  {
    internalState_.packetNumber = latestData_.packetNumber;
    internalState_.timestampOfLastMessage = latestDataTimestamp_;
    internalState_.playersPerTeam = latestData_.playersPerTeam;
    // The static_casts should be OK because the enums are defined to the macros from
    // RoboCupGameControlData.h.
    internalState_.type = static_cast<CompetitionType>(latestData_.competitionType);
    internalState_.competitionPhase = static_cast<CompetitionPhase>(latestData_.competitionPhase);
    internalState_.gamePhase = static_cast<GamePhase>(latestData_.gamePhase);

    const auto newState = static_cast<GameState>(latestData_.state);
    if (newState != internalState_.gameState)
    {
      internalState_.gameState = newState;
      if (newState == GameState::PLAYING && internalState_.gamePhase == GamePhase::NORMAL)
      {
        // GameController sends playing with a delay of 15s
        internalState_.gameStateChanged = cycleInfo_->startTime - std::chrono::seconds(15);
      }
      else
      {
        internalState_.gameStateChanged = cycleInfo_->startTime;
      }
    }

    const auto newSetPlay = static_cast<SetPlay>(latestData_.setPlay);
    if (newSetPlay != internalState_.setPlay)
    {
      internalState_.setPlay = static_cast<SetPlay>(latestData_.setPlay);
      internalState_.setPlayChanged = cycleInfo_->startTime;
    }

    internalState_.firstHalf = latestData_.firstHalf != 0;
    internalState_.kickingTeam = latestData_.kickingTeam == playerConfiguration_->teamNumber;
    internalState_.kickingTeamNumber = latestData_.kickingTeam;
    internalState_.secondaryTime = latestData_.secondaryTime;
//    internalState_.dropInTeam = latestData_.dropInTeam;
//    internalState_.dropInTime = latestData_.dropInTime;
    internalState_.remainingTime = latestData_.secsRemaining;
    internalState_.teamColor = static_cast<TeamColor>(latestData_.teams[teamIndex_].teamColour);
    internalState_.score = latestData_.teams[teamIndex_].score;
    if (playerConfiguration_->playerNumber <= MAX_NUM_PLAYERS)
    {
      internalState_.penalty = static_cast<Penalty>(
          latestData_.teams[teamIndex_].players[playerConfiguration_->playerNumber - 1].penalty);
      internalState_.remainingPenaltyTime = latestData_.teams[teamIndex_]
                                                .players[playerConfiguration_->playerNumber - 1]
                                                .secsTillUnpenalised;
    }
    for (unsigned int i = 0; i < MAX_NUM_PLAYERS; i++)
    {
      internalState_.penalties[i] =
          static_cast<Penalty>(latestData_.teams[teamIndex_].players[i].penalty);
    }
  }
  catch (const std::exception& e)
  {
    print(e.what(), LogLevel::ERROR);
  }
}

void GameController::handleChestButton()
{
  if (buttonData_->lastChestButtonDoublePress > lastChestButtonDoublePress_)
  {
    // Double tap completely resets the game state so nothing is remembered from previous network
    // messages.
    internalState_.packetNumber = 0;
    internalState_.timestampOfLastMessage = cycleInfo_->startTime;
    internalState_.playersPerTeam = 5;
    internalState_.type = CompetitionType::NORMAL;
    internalState_.competitionPhase = CompetitionPhase::ROUNDROBIN;
    internalState_.gameState = GameState::INITIAL;
    internalState_.gameStateChanged = cycleInfo_->startTime;
    internalState_.gamePhase = GamePhase::NORMAL;
    internalState_.setPlay = SetPlay::NONE;
    internalState_.setPlayChanged = cycleInfo_->startTime;
    internalState_.firstHalf = true;
    internalState_.kickingTeam = true;
    internalState_.kickingTeamNumber = static_cast<uint8_t>(playerConfiguration_->teamNumber);
    internalState_.secondaryTime = 0.f;
//    internalState_.dropInTeam = 0;
//    internalState_.dropInTime = -1.f;
    internalState_.remainingTime = 600.f;
    internalState_.teamColor = TeamColor::BLUE;
    internalState_.score = 0;
    internalState_.penalty = Penalty::NONE;
    internalState_.penalties.resize(MAX_NUM_PLAYERS, Penalty::NONE);
    internalState_.remainingPenaltyTime = 0.0f;
    internalState_.chestButtonWasPressedInInitial = false;
    lastChestButtonDoublePress_ = buttonData_->lastChestButtonDoublePress;
  }
  else if (buttonData_->lastChestButtonSinglePress > lastChestButtonSinglePress_)
  {
    if (internalState_.gameState == GameState::INITIAL &&
        !internalState_.chestButtonWasPressedInInitial)
    {
      internalState_.chestButtonWasPressedInInitial = true;
      if (forcePenaltyShootout_())
      {
        internalState_.gamePhase = GamePhase::PENALTYSHOOT;
        // Even numbers become strikers, odd numbers become keeper.
        internalState_.kickingTeam = (playerConfiguration_->playerNumber % 2) == 0;
      }
    }
    else
    {
      if (cycleInfo_->getTimeDiff(latestDataTimestamp_) <= 2.f)
      {
        // game controller is active. Ignore chest button.
        Log(LogLevel::INFO) << "Chest button pressed with active gameController. Ignoring...";
      }
      else
      {
        if (internalState_.penalty == Penalty::NONE)
        {
          internalState_.penalty = Penalty::MANUAL;
          Log(LogLevel::INFO) << "Manually penalized (no active gameController detected)";
        }
        else
        {
          internalState_.penalty = Penalty::NONE;
          // If no GameController message has been received in the last 2 seconds (== no
          // GameController is active), then it is assumed that either testing without
          // GameController is inteded or button interface is used because the WiFi broken. In that
          // case, the state is switched to playing because this is wanted.
          internalState_.gameState = GameState::PLAYING;
          Log(LogLevel::INFO) << "Manually unpenalized (no active gameController detected)";
        }
      }
    }
    lastChestButtonSinglePress_ = buttonData_->lastChestButtonSinglePress;
  }
}
