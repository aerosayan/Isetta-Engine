/*
 * Copyright (c) 2018 Isetta
 */

#include "Networking/NetworkTransform.h"

#include <algorithm>
#include <unordered_map>
#include "Core/Time/Time.h"
#include "Networking/NetworkId.h"
#include "Scene/Entity.h"
#include "Scene/Level.h";
#include "Scene/LevelManager.h"
#include "Scene/Transform.h"

namespace Isetta {

bool NetworkTransform::registeredCallbacks = false;
std::unordered_map<int, float> NetworkTransform::serverPosTimestamps;
std::unordered_map<int, float> NetworkTransform::serverRotTimestamps;
std::unordered_map<int, float> NetworkTransform::serverScaleTimestamps;

void NetworkTransform::Start() {
  if (!registeredCallbacks) {
    // Position callbacks
    NetworkManager::Instance().RegisterClientCallback<PositionMessage>(
        [&](yojimbo::Message* message) {
          PositionMessage* positionMessage =
              reinterpret_cast<PositionMessage*>(message);

          NetworkId* netId =
              NetworkManager::Instance().GetNetworkId(positionMessage->netId);
          if (!netId || netId->HasClientAuthority()) {
            return;
          }

          Entity* entity = netId->entity;
          NetworkTransform* nt = entity->GetComponent<NetworkTransform>();

          if (nt && positionMessage->timestamp < nt->lastPosMessage) {
            return;
          }

          if (entity) {
            Transform* t = entity->transform;
            nt->targetPos = positionMessage->localPos;

            nt->posInterpolation = 0;

            if ((Math::Vector3::Scale(t->GetWorldScale(),
                                      t->GetLocalPos() - nt->targetPos))
                    .SqrMagnitude() >= nt->snapDistance * nt->snapDistance) {
              t->SetLocalPos(positionMessage->localPos);
              nt->posInterpolation = 1;
            }

            nt->prevPos = t->GetLocalPos();
            nt->lastPosMessage = positionMessage->timestamp;
          }
        });

    NetworkManager::Instance().RegisterServerCallback<PositionMessage>(
        [](int clientIdx, yojimbo::Message* message) {
          PositionMessage* positionMessage =
              reinterpret_cast<PositionMessage*>(message);

          if (serverScaleTimestamps[positionMessage->netId] <=
              positionMessage->timestamp) {
            serverScaleTimestamps[positionMessage->netId] =
                positionMessage->timestamp;
            NetworkManager::Instance()
                .SendAllMessageFromServer<PositionMessage>(positionMessage);
          }
        });

    // Rotation callbacks
    NetworkManager::Instance().RegisterClientCallback<RotationMessage>(
        [&](yojimbo::Message* message) {
          RotationMessage* rotationMessage =
              reinterpret_cast<RotationMessage*>(message);

          NetworkId* netId =
              NetworkManager::Instance().GetNetworkId(rotationMessage->netId);
          if (!netId || netId->HasClientAuthority()) {
            return;
          }

          Entity* entity = netId->entity;
          NetworkTransform* nt = entity->GetComponent<NetworkTransform>();

          if (nt && rotationMessage->timestamp < nt->lastRotMessage) {
            return;
          }

          if (entity) {
            Transform* t = entity->transform;
            nt->targetRot = rotationMessage->localRot;

            nt->rotInterpolation = 0;

            if (abs(Math::Quaternion::AngleDeg(
                    t->GetLocalRot(), nt->targetRot)) >= nt->snapRotation) {
              t->SetLocalRot(rotationMessage->localRot);
              nt->rotInterpolation = 1;
            }

            nt->prevRot = t->GetLocalRot();
            nt->lastRotMessage = rotationMessage->timestamp;
          }
        });

    NetworkManager::Instance().RegisterServerCallback<RotationMessage>(
        [](int clientIdx, yojimbo::Message* message) {
          RotationMessage* rotationMessage =
              reinterpret_cast<RotationMessage*>(message);

          if (serverScaleTimestamps[rotationMessage->netId] <=
              rotationMessage->timestamp) {
            serverScaleTimestamps[rotationMessage->netId] =
                rotationMessage->timestamp;
            NetworkManager::Instance()
                .SendAllMessageFromServer<RotationMessage>(rotationMessage);
          }
        });

    // Scale callbacks
    NetworkManager::Instance().RegisterClientCallback<ScaleMessage>(
        [&](yojimbo::Message* message) {
          ScaleMessage* scaleMessage = reinterpret_cast<ScaleMessage*>(message);

          NetworkId* netId =
              NetworkManager::Instance().GetNetworkId(scaleMessage->netId);
          if (!netId || netId->HasClientAuthority()) {
            return;
          }

          Entity* entity = netId->entity;
          NetworkTransform* nt = entity->GetComponent<NetworkTransform>();

          if (nt && scaleMessage->timestamp < nt->lastScaleMessage) {
            return;
          }

          if (entity) {
            Transform* t = entity->transform;
            nt->targetScale = scaleMessage->localScale;

            nt->scaleInterpolation = 0;

            if ((t->GetLocalScale() - nt->targetScale).SqrMagnitude() >=
                nt->snapScale * nt->snapScale) {
              t->SetLocalScale(scaleMessage->localScale);
              nt->scaleInterpolation = 1;
            }

            nt->prevScale = t->GetLocalScale();
            nt->lastScaleMessage = scaleMessage->timestamp;
          }
        });

    NetworkManager::Instance().RegisterServerCallback<ScaleMessage>(
        [](int clientIdx, yojimbo::Message* message) {
          ScaleMessage* scaleMessage = reinterpret_cast<ScaleMessage*>(message);

          if (serverScaleTimestamps[scaleMessage->netId] <=
              scaleMessage->timestamp) {
            serverScaleTimestamps[scaleMessage->netId] =
                scaleMessage->timestamp;
            NetworkManager::Instance().SendAllMessageFromServer<ScaleMessage>(
                scaleMessage);
          }
        });

    // Transform callbacks
    NetworkManager::Instance().RegisterClientCallback<TransformMessage>(
        [&](yojimbo::Message* message) {
          TransformMessage* transformMessage =
              reinterpret_cast<TransformMessage*>(message);

          NetworkId* netId =
              NetworkManager::Instance().GetNetworkId(transformMessage->netId);
          if (!netId || netId->HasClientAuthority()) {
            return;
          }

          Entity* entity = netId->entity;
          NetworkTransform* nt = entity->GetComponent<NetworkTransform>();

          if (!nt) {
            return;
          }

          // Snapping
          if (transformMessage->snap) {
            Transform* t = entity->transform;

            // Position
            if (transformMessage->timestamp >= nt->lastPosMessage) {
              t->SetLocalPos(transformMessage->localPos);
              nt->prevPos = t->GetLocalPos();
              nt->lastPosMessage = transformMessage->timestamp;
            }

            // Rotation
            if (transformMessage->timestamp >= nt->lastRotMessage) {
              t->SetLocalRot(transformMessage->localRot);
              nt->prevRot = t->GetLocalRot();
              nt->lastRotMessage = transformMessage->timestamp;
            }

            // Scale
            if (transformMessage->timestamp >= nt->lastScaleMessage) {
              t->SetLocalScale(transformMessage->localScale);
              nt->prevScale = t->GetLocalScale();
              nt->lastScaleMessage = transformMessage->timestamp;
            }

            nt->posInterpolation = 1;
            nt->rotInterpolation = 1;
            nt->scaleInterpolation = 1;
          } else {  // Not snapping
            Transform* t = entity->transform;

            // Position
            if (transformMessage->timestamp >= nt->lastPosMessage) {
              nt->targetPos = transformMessage->localPos;

              nt->posInterpolation = 0;

              if ((Math::Vector3::Scale(t->GetWorldScale(),
                                        t->GetLocalPos() - nt->targetPos))
                      .SqrMagnitude() >= nt->snapDistance * nt->snapDistance) {
                t->SetLocalPos(transformMessage->localPos);
                nt->posInterpolation = 1;
              }

              nt->prevPos = t->GetLocalPos();
              nt->lastPosMessage = transformMessage->timestamp;
            }

            // Rotation
            if (transformMessage->timestamp >= nt->lastRotMessage) {
              nt->targetRot = transformMessage->localRot;

              nt->rotInterpolation = 0;

              if (abs(Math::Quaternion::AngleDeg(
                      t->GetLocalRot(), nt->targetRot)) >= nt->snapRotation) {
                t->SetLocalRot(transformMessage->localRot);
                nt->rotInterpolation = 1;
              }

              nt->prevRot = t->GetLocalRot();
              nt->lastRotMessage = transformMessage->timestamp;
            }

            // Scale
            if (transformMessage->timestamp >= nt->lastScaleMessage) {
              nt->targetScale = transformMessage->localScale;

              nt->scaleInterpolation = 0;

              if ((t->GetLocalScale() - nt->targetScale).SqrMagnitude() >=
                  nt->snapScale * nt->snapScale) {
                t->SetLocalScale(transformMessage->localScale);
                nt->scaleInterpolation = 1;
              }

              nt->prevScale = t->GetLocalScale();
              nt->lastScaleMessage = transformMessage->timestamp;
            }
          }
        });

    NetworkManager::Instance().RegisterServerCallback<TransformMessage>(
        [](int clientIdx, yojimbo::Message* message) {
          TransformMessage* transformMessage =
              reinterpret_cast<TransformMessage*>(message);

          NetworkManager::Instance().SendAllMessageFromServer<TransformMessage>(
              transformMessage);
        });

    // Parenting callbacks
    NetworkManager::Instance().RegisterClientCallback<ParentMessage>(
        [](yojimbo::Message* message) {
          ParentMessage* parentMessage =
              reinterpret_cast<ParentMessage*>(message);

          NetworkId* netId =
              NetworkManager::Instance().GetNetworkId(parentMessage->netId);
          if (!netId) {
            return;
          }

          Entity* entity = netId->entity;

          if (parentMessage->parentNetId == 0) {
            entity->transform->SetParent(nullptr);
          } else {
            Entity* parentEntity = NetworkManager::Instance().GetNetworkEntity(
                parentMessage->parentNetId);
            entity->transform->SetParent(parentEntity->transform);
          }
        });

    NetworkManager::Instance().RegisterServerCallback<ParentMessage>(
        [](int clientIdx, yojimbo::Message* message) {
          ParentMessage* parentMessage =
              reinterpret_cast<ParentMessage*>(message);

          NetworkManager::Instance()
              .SendAllButClientMessageFromServer<ParentMessage>(clientIdx,
                                                                parentMessage);
        });

    NetworkTransform::registeredCallbacks = true;
  }
  netId = entity->GetComponent<NetworkId>();
  targetPos = entity->transform->GetLocalPos();
  prevPos = targetPos;
  targetRot = entity->transform->GetLocalRot();
  prevRot = targetRot;
  targetScale = entity->transform->GetLocalScale();
  prevScale = targetScale;

  lastPosMessage = 0;
  lastRotMessage = 0;
  lastScaleMessage = 0;
}

void NetworkTransform::Update() {
  if (posInterpolation < 1 || rotInterpolation < 1 || scaleInterpolation < 1) {
    Transform* t = entity->transform;

    // TODO(Caleb): Find a way to make this more consistent (netId->updateInterval isn't necessarily synced, and maxFPS does not guarantee number of fixed update frames)
    float netIdLerp =
        netId->updateInterval / (float)Config::Instance().loopConfig.maxFps.GetVal();

    // Translation
    posInterpolation =
        min(posInterpolation + Time::GetDeltaTime() / netIdLerp, 1);
    t->SetLocalPos(
        Math::Vector3::Lerp(prevPos, targetPos, posInterpolation));
    // Rotation
    rotInterpolation =
        min(rotInterpolation + Time::GetDeltaTime() / netIdLerp, 1);
    t->SetLocalRot(Math::Quaternion::Slerp(prevRot, targetRot,
                                           rotInterpolation));
    // Scale
    scaleInterpolation =
        min(scaleInterpolation + Time::GetDeltaTime() / netIdLerp, 1);
    t->SetLocalScale(Math::Vector3::Lerp(prevScale, targetScale,
                                         scaleInterpolation));
  }
}

void NetworkTransform::FixedUpdate() {
  if (netId->HasClientAuthority()) {
    ++updateCounter;

    if (updateCounter >= netId->updateInterval) {
      updateCounter = 0;

      Transform* t = entity->transform;
      // Position
      if (Math::Vector3::Scale(t->GetParent()->GetWorldScale(),
                               t->GetLocalPos() - prevPos)
              .SqrMagnitude() >= updateDistance * updateDistance) {
        PositionMessage* message =
            NetworkManager::Instance()
                .GenerateMessageFromClient<PositionMessage>();
        prevPos = t->GetLocalPos();
        message->timestamp = Time::GetElapsedTime();
        message->localPos = t->GetLocalPos();
        message->netId = netId->id;
        NetworkManager::Instance().SendMessageFromClient(message);
      }
      // Rotation
      if (Math::Quaternion::AngleDeg(t->GetLocalRot(), prevRot) >=
          updateRotation) {
        RotationMessage* message =
            NetworkManager::Instance()
                .GenerateMessageFromClient<RotationMessage>();
        prevRot = t->GetLocalRot();
        message->timestamp = Time::GetElapsedTime();
        message->localRot = t->GetLocalRot();
        message->netId = netId->id;
        NetworkManager::Instance().SendMessageFromClient(message);
      }
      // Scale
      if ((t->GetLocalScale() - prevScale).SqrMagnitude() >=
          updateScale * updateScale) {
        ScaleMessage* message = NetworkManager::Instance()
                                    .GenerateMessageFromClient<ScaleMessage>();
        prevScale = t->GetLocalScale();
        message->timestamp = Time::GetElapsedTime();
        message->localScale = t->GetLocalScale();
        message->netId = netId->id;
        NetworkManager::Instance().SendMessageFromClient(message);
      }
    }
  }
}

void NetworkTransform::SnapLocalTransform() {
  Transform* t = entity->transform;
  t->SetLocalPos(targetPos);
  t->SetLocalRot(targetRot);
  t->SetLocalScale(targetScale);

  prevPos = targetPos;
  prevRot = targetRot;
  prevScale = targetScale;

  posInterpolation = 1;
  rotInterpolation = 1;
  scaleInterpolation = 1;
}

void NetworkTransform::ForceSendTransform(bool snap) {
  if (netId->HasClientAuthority()) {
    Transform* t = entity->transform;
    prevPos = t->GetLocalPos();
    prevRot = t->GetLocalRot();
    prevScale = t->GetLocalScale();

    TransformMessage* message =
        NetworkManager::Instance()
            .GenerateMessageFromClient<TransformMessage>();

    message->timestamp = Time::GetElapsedTime();
    message->snap = snap;
    message->localPos = t->GetLocalPos();
    message->localRot = t->GetLocalRot();
    message->localScale = t->GetLocalScale();
    message->netId = netId->id;

    NetworkManager::Instance().SendMessageFromClient(message);
  }
}

void NetworkTransform::SetNetworkedParentToRoot() {
  ParentMessage* message =
      NetworkManager::Instance().GenerateMessageFromClient<ParentMessage>();
  message->netId = netId->id;
  message->parentNetId = 0;
  NetworkManager::Instance().SendMessageFromClient(message);

  entity->transform->SetParent(nullptr);
}

bool NetworkTransform::SetNetworkedParent(int networkId) {
  Entity* parent = NetworkManager::Instance().GetNetworkEntity(networkId);
  if (!parent) {
    return false;
  }

  ParentMessage* message =
      NetworkManager::Instance().GenerateMessageFromClient<ParentMessage>();
  message->netId = netId->id;
  message->parentNetId = networkId;
  NetworkManager::Instance().SendMessageFromClient(message);

  entity->transform->SetParent(parent->transform);
  return true;
}
}  // namespace Isetta
