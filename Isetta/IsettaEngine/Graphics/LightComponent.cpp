/*
 * Copyright (c) 2018 Isetta
 */
#include "Graphics/LightComponent.h"

#include "Core/Config/Config.h"
#include "Core/Math/Vector4.h"
#include "Scene/Entity.h"
#include "Scene/Transform.h"
#include "Util.h"
#include "brofiler/ProfilerCore/Brofiler.h"

namespace Isetta {

RenderModule* LightComponent::renderModule{nullptr};

LightComponent::LightComponent() : name{entity->GetEntityIdString()} {
  ASSERT(renderModule != nullptr);
  renderModule->lightComponents.push_back(this);

  renderResource = LoadResourceFromFile(
      Config::Instance().lightConfig.lightMaterial.GetVal());
}

H3DRes LightComponent::LoadResourceFromFile(std::string_view resourceName) {
  H3DRes lightMatRes =
      h3dAddResource(H3DResTypes::Material, resourceName.data(), 0);

  RenderModule::LoadResourceFromDisk(
      lightMatRes, Util::StrFormat("LightComponent::LoadResourceFromFile => "
                                   "Cannot load the resource from %s",
                                   resourceName.data()));

  return lightMatRes;
}

void LightComponent::Start() {
  SetProperty<Property::RADIUS>(CONFIG_M_VAL(lightConfig, radius));
  SetProperty<Property::FOV>(CONFIG_M_VAL(lightConfig, fieldOfView));
  SetProperty<Property::COLOR>(
      Color{Math::Vector4(CONFIG_M_VAL(lightConfig, color), 1.0f)});
  SetProperty<Property::COLOR_MULTIPLIER>(
      CONFIG_M_VAL(lightConfig, colorMultiplier));
  SetProperty<Property::SHADOW_MAP_COUNT>(
      CONFIG_M_VAL(lightConfig, shadowMapCount));
  SetProperty<Property::SHADOW_MAP_BIAS>(
      CONFIG_M_VAL(lightConfig, shadowMapBias));
}

void LightComponent::OnEnable() {
  if (renderNode == 0) {
    renderNode = h3dAddLightNode(H3DRootNode, name.data(), renderResource,
                                 "LIGHTING", "SHADOWMAP");
  } else {
    h3dSetNodeFlags(renderNode, 0, true);
  }
}

void LightComponent::OnDisable() {
  h3dSetNodeFlags(renderNode, H3DNodeFlags::Inactive, true);
}

void LightComponent::OnDestroy() {
  h3dRemoveNode(renderNode);
  renderModule->lightComponents.remove(this);
}

void LightComponent::UpdateH3DTransform() const {
  PROFILE
  Transform::SetH3DNodeTransform(renderNode, *transform);
}
}  // namespace Isetta
