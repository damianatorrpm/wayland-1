/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QWaylandClient>

#include "wlroutputmanagerv1_p.h"
#include "logging_p.h"

WlrOutputManagerV1Private::WlrOutputManagerV1Private(WlrOutputManagerV1 *self)
    : QtWaylandServer::zwlr_output_manager_v1()
    , q_ptr(self)
{
}

WlrOutputManagerV1Private::~WlrOutputManagerV1Private()
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_finished(resource->handle);
}

void WlrOutputManagerV1Private::registerHead(WlrOutputHeadV1 *head)
{
    Q_Q(WlrOutputManagerV1);

    if (heads.contains(head))
        return;

    heads.append(head);

    Q_EMIT q->headAdded(head);
}

void WlrOutputManagerV1Private::zwlr_output_manager_v1_bind_resource(QtWaylandServer::zwlr_output_manager_v1::Resource *resource)
{
    if (!compositor)
        return;

    // Send all the heads at once when the client binds
    for (auto *head : qAsConst(heads)) {
        auto *headPrivate = WlrOutputHeadV1Private::get(head);
        auto *headResource = headPrivate->add(resource->client(), headPrivate->interfaceVersion());
        send_head(resource->handle, headResource->handle);
        headPrivate->sendInfo(headResource);
    }
    send_done(resource->handle, compositor->nextSerial());
}

void WlrOutputManagerV1Private::zwlr_output_manager_v1_create_configuration(QtWaylandServer::zwlr_output_manager_v1::Resource *resource, uint32_t id, uint32_t serial)
{
    Q_Q(WlrOutputManagerV1);

    if (stoppedClients.contains(resource->client()))
        return;

    auto version = WlrOutputConfigurationV1Private::interfaceVersion();
    auto *configuration = new WlrOutputConfigurationV1(q);
    WlrOutputConfigurationV1Private::get(configuration)->manager = q;
    WlrOutputConfigurationV1Private::get(configuration)->add(resource->client(), id, version);
    configurations[serial] = configuration;

    Q_EMIT q->configurationCreated(configuration);
}

void WlrOutputManagerV1Private::zwlr_output_manager_v1_stop(QtWaylandServer::zwlr_output_manager_v1::Resource *resource)
{
    Q_Q(WlrOutputManagerV1);

    stoppedClients.append(resource->client());

    Q_EMIT q->clientStopped(QWaylandClient::fromWlClient(compositor, resource->client()));
}


WlrOutputManagerV1::WlrOutputManagerV1()
    : QWaylandCompositorExtensionTemplate<WlrOutputManagerV1>()
    , d_ptr(new WlrOutputManagerV1Private(this))
{
}

WlrOutputManagerV1::WlrOutputManagerV1(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<WlrOutputManagerV1>(compositor)
    , d_ptr(new WlrOutputManagerV1Private(this))
{
}

WlrOutputManagerV1::~WlrOutputManagerV1()
{
    delete d_ptr;
}

void WlrOutputManagerV1::initialize()
{
    Q_D(WlrOutputManagerV1);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(lcWaylandServer) << "Failed to find QWaylandCompositor when initializing WlrOutputManagerV1";
        return;
    }
    d->compositor = compositor;
    d->init(compositor->display(), WlrOutputManagerV1Private::interfaceVersion());
}

QWaylandCompositor *WlrOutputManagerV1::compositor() const
{
    Q_D(const WlrOutputManagerV1);
    return d->compositor;
}

QVector<WlrOutputHeadV1 *> WlrOutputManagerV1::heads() const
{
    Q_D(const WlrOutputManagerV1);
    return d->heads;
}

void WlrOutputManagerV1::done(quint32 serial)
{
    Q_D(WlrOutputManagerV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_done(resource->handle, serial);
}

void WlrOutputManagerV1::finished()
{
    Q_D(WlrOutputManagerV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_finished(resource->handle);
}

const wl_interface *WlrOutputManagerV1::interface()
{
    return WlrOutputManagerV1Private::interface();
}

QByteArray WlrOutputManagerV1::interfaceName()
{
    return WlrOutputManagerV1Private::interfaceName();
}


WlrOutputHeadV1Private::WlrOutputHeadV1Private(WlrOutputHeadV1 *self)
    : QtWaylandServer::zwlr_output_head_v1()
    , q_ptr(self)
{
}

WlrOutputHeadV1Private::~WlrOutputHeadV1Private()
{
    qDeleteAll(modes);
    modes.clear();

    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_finished(resource->handle);
}

void WlrOutputHeadV1Private::sendInfo(Resource *resource)
{
    Q_Q(WlrOutputHeadV1);

    modesSent = true;

    send_name(resource->handle, name);
    send_description(resource->handle, description);
    if (physicalSize.width() > 0 && physicalSize.height() > 0)
        send_physical_size(resource->handle, physicalSize.width(), physicalSize.height());
    send_enabled(resource->handle, enabled ? 1 : 0);
    if (enabled) {
        send_position(resource->handle, position.x(), position.y());
        send_transform(resource->handle, static_cast<int32_t>(transform));
        send_scale(resource->handle, wl_fixed_from_double(scale));
    }

    for (auto *mode : qAsConst(modes)) {
        auto *modePrivate = WlrOutputModeV1Private::get(mode);
        auto *modeResource = modePrivate->add(resource->client(), modePrivate->interfaceVersion());
        send_mode(resource->handle, modeResource->handle);
        modePrivate->send_size(modeResource->handle, modePrivate->size.width(), modePrivate->size.height());
        modePrivate->send_refresh(modeResource->handle, modePrivate->refreshRate);

        if (enabled && currentMode) {
            if (mode == currentMode)
                send_current_mode(resource->handle, modeResource->handle);
        }

        if (enabled && preferredMode) {
            if (mode == preferredMode)
                modePrivate->send_preferred(modeResource->handle);
        }
    }
}

WlrOutputHeadV1 *WlrOutputHeadV1Private::fromResource(wl_resource *resource)
{
    return static_cast<WlrOutputHeadV1Private *>(WlrOutputHeadV1Private::Resource::fromResource(resource)->zwlr_output_head_v1_object)->q_func();
}


WlrOutputHeadV1::WlrOutputHeadV1(QObject *parent)
    : QObject(parent)
    , d_ptr(new WlrOutputHeadV1Private(this))
{
}

WlrOutputHeadV1::~WlrOutputHeadV1()
{
    Q_D(WlrOutputHeadV1);

    if (d->manager)
        WlrOutputManagerV1Private::get(d->manager)->heads.removeOne(this);

    delete d_ptr;
}

bool WlrOutputHeadV1::isInitialized() const
{
    Q_D(const WlrOutputHeadV1);
    return d->initialized;
}

void WlrOutputHeadV1::initialize()
{
    Q_D(WlrOutputHeadV1);

    if (d->manager) {
        WlrOutputManagerV1Private::get(d->manager)->registerHead(this);
        d->initialized = true;
    }
}

WlrOutputManagerV1 *WlrOutputHeadV1::manager() const
{
    Q_D(const WlrOutputHeadV1);
    return d->manager;
}

void WlrOutputHeadV1::setManager(WlrOutputManagerV1 *manager)
{
    Q_D(WlrOutputHeadV1);

    if (d->manager) {
        qCWarning(lcWaylandServer, "Output manager already set, cannot override");
        return;
    }

    d->manager = manager;
    Q_EMIT managerChanged();
}

bool WlrOutputHeadV1::isEnabled() const
{
    Q_D(const WlrOutputHeadV1);
    return d->enabled;
}

void WlrOutputHeadV1::setEnabled(bool enabled)
{
    Q_D(WlrOutputHeadV1);

    if (d->enabled == enabled)
        return;

    d->enabled = enabled;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_enabled(resource->handle, enabled ? 1 : 0);
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT enabledChanged();
}

QString WlrOutputHeadV1::name() const
{
    Q_D(const WlrOutputHeadV1);
    return d->name;
}

void WlrOutputHeadV1::setName(const QString &name)
{
    Q_D(WlrOutputHeadV1);

    // Can only be changed the first time
    if (d->nameChanged) {
        qCWarning(lcWaylandServer, "The name does not change over the lifetime of the output head");
        return;
    }

    if (d->name == name)
        return;

    d->name = name;
    d->nameChanged = true;

    Q_EMIT nameChanged();
}

QString WlrOutputHeadV1::description() const
{
    Q_D(const WlrOutputHeadV1);
    return d->name;
}

void WlrOutputHeadV1::setDescription(const QString &description)
{
    Q_D(WlrOutputHeadV1);

    // Can only be changed the first time
    if (d->descriptionChanged) {
        qCWarning(lcWaylandServer, "The description does not change over the lifetime of the output head");
        return;
    }

    if (d->description == description)
        return;

    d->description = description;
    d->descriptionChanged = true;

    Q_EMIT descriptionChanged();
}

QSize WlrOutputHeadV1::physicalSize() const
{
    Q_D(const WlrOutputHeadV1);
    return d->physicalSize;
}

void WlrOutputHeadV1::setPhysicalSize(const QSize &physicalSize)
{
    Q_D(WlrOutputHeadV1);

    if (d->physicalSize == physicalSize)
        return;

    d->physicalSize = physicalSize;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_physical_size(resource->handle, physicalSize.width(), physicalSize.height());
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT physicalSizeChanged();
}

QPoint WlrOutputHeadV1::position() const
{
    Q_D(const WlrOutputHeadV1);
    return d->position;
}

void WlrOutputHeadV1::setPosition(const QPoint &position)
{
    Q_D(WlrOutputHeadV1);

    if (!d->enabled) {
        qCWarning(lcWaylandServer, "Cannot change position on a disabled output head");
        return;
    }

    if (d->position == position)
        return;

    d->position = position;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_position(resource->handle, position.x(), position.y());
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT positionChanged();
}

QVector<WlrOutputModeV1 *> WlrOutputHeadV1::modes() const
{
    Q_D(const WlrOutputHeadV1);
    return d->modes;
}

void WlrOutputHeadV1::addMode(WlrOutputModeV1 *mode)
{
    Q_D(WlrOutputHeadV1);

    if (d->modesSent) {
        qCWarning(lcWaylandServer, "Cannot add new modes after initialization");
        return;
    }

    d->modes.append(mode);

    Q_EMIT modeAdded(mode);
    Q_EMIT modesChanged();
}

WlrOutputModeV1 *WlrOutputHeadV1::currentMode() const
{
    Q_D(const WlrOutputHeadV1);
    return d->currentMode;
}

void WlrOutputHeadV1::setCurrentMode(WlrOutputModeV1 *mode)
{
    Q_D(WlrOutputHeadV1);

    if (!d->modes.contains(mode)) {
        qCWarning(lcWaylandServer, "Failed to set current mode: add modes first");
        return;
    }

    if (d->currentMode == mode)
        return;

    d->currentMode = mode;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_current_mode(resource->handle, WlrOutputModeV1Private::get(mode)->resource()->handle);
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT currentModeChanged();
}

WlrOutputModeV1 *WlrOutputHeadV1::preferredMode() const
{
    Q_D(const WlrOutputHeadV1);
    return d->preferredMode;
}

void WlrOutputHeadV1::setPreferredMode(WlrOutputModeV1 *mode)
{
    Q_D(WlrOutputHeadV1);

    if (!d->modes.contains(mode)) {
        qCWarning(lcWaylandServer, "Failed to set preferred mode: add modes first");
        return;
    }

    if (d->preferredMode == mode)
        return;

    d->preferredMode = mode;

    if (d->initialized) {
        auto *modePrivate = WlrOutputModeV1Private::get(mode);
        const auto values = modePrivate->resourceMap().values();
        for (auto *resource : values)
            modePrivate->send_preferred(resource->handle);
    }

    Q_EMIT preferredModeChanged();
}

QWaylandOutput::Transform WlrOutputHeadV1::transform() const
{
    Q_D(const WlrOutputHeadV1);
    return d->transform;
}

void WlrOutputHeadV1::setTransform(QWaylandOutput::Transform transform)
{
    Q_D(WlrOutputHeadV1);

    if (!d->enabled) {
        qCWarning(lcWaylandServer, "Cannot change transform on a disabled output head");
        return;
    }

    if (d->transform == transform)
        return;

    d->transform = transform;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_transform(resource->handle, static_cast<int32_t>(transform));
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT transformChanged();
}

qreal WlrOutputHeadV1::scale() const
{
    Q_D(const WlrOutputHeadV1);
    return d->scale;
}

void WlrOutputHeadV1::setScale(qreal scale)
{
    Q_D(WlrOutputHeadV1);

    if (!d->enabled) {
        qCWarning(lcWaylandServer, "Cannot change scale factor on a disabled output head");
        return;
    }

    if (d->scale == scale)
        return;

    d->scale = scale;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_scale(resource->handle, wl_fixed_from_double(scale));
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT scaleChanged();
}



WlrOutputHeadV1Qml::WlrOutputHeadV1Qml(QObject *parent)
    : WlrOutputHeadV1(parent)
{
}

QQmlListProperty<WlrOutputModeV1> WlrOutputHeadV1Qml::modesList()
{
    auto countFunc = [](QQmlListProperty<WlrOutputModeV1> *prop) {
        return static_cast<WlrOutputHeadV1 *>(prop->object)->modes().size();
    };
    auto atFunc = [](QQmlListProperty<WlrOutputModeV1> *prop, int index) {
        return static_cast<WlrOutputHeadV1 *>(prop->object)->modes().at(index);
    };
    return QQmlListProperty<WlrOutputModeV1>(this, this, countFunc, atFunc);
}

void WlrOutputHeadV1Qml::componentComplete()
{
    if (!isInitialized()) {
        initialize();

        if (!isInitialized())
            qCWarning(lcWaylandServer,
                      "Unable to find WlrOutputManagerV1: %p head will not be registered",
                      this);
    }
}


WlrOutputModeV1Private::WlrOutputModeV1Private(WlrOutputModeV1 *self)
    : QtWaylandServer::zwlr_output_mode_v1()
    , q_ptr(self)
{
}

WlrOutputModeV1Private::~WlrOutputModeV1Private()
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_finished(resource->handle);
}

WlrOutputModeV1 *WlrOutputModeV1Private::fromResource(wl_resource *resource)
{
    return static_cast<WlrOutputModeV1Private *>(WlrOutputModeV1Private::Resource::fromResource(resource)->zwlr_output_mode_v1_object)->q_func();
}


WlrOutputModeV1::WlrOutputModeV1(QObject *parent)
    : QObject(parent)
    , d_ptr(new WlrOutputModeV1Private(this))
{
}

bool WlrOutputModeV1::isInitialized() const
{
    Q_D(const WlrOutputModeV1);
    return d->initialized;
}

QSize WlrOutputModeV1::size() const
{
    Q_D(const WlrOutputModeV1);
    return d->size;
}

void WlrOutputModeV1::setSize(const QSize &size)
{
    Q_D(WlrOutputModeV1);

    if (d->size == size)
        return;

    d->size = size;

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_size(resource->handle, size.width(), size.height());

    Q_EMIT sizeChanged();
}

qint32 WlrOutputModeV1::refresh() const
{
    Q_D(const WlrOutputModeV1);
    return d->refreshRate;
}

void WlrOutputModeV1::setRefresh(qint32 refreshRate)
{
    Q_D(WlrOutputModeV1);

    if (d->refreshRate == refreshRate)
        return;

    d->refreshRate = refreshRate;

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_refresh(resource->handle, refreshRate);

    Q_EMIT refreshChanged();
}


WlrOutputConfigurationHeadV1Private::WlrOutputConfigurationHeadV1Private(WlrOutputConfigurationHeadV1 *self)
    : QtWaylandServer::zwlr_output_configuration_head_v1()
    , q_ptr(self)
{
}

void WlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_mode(Resource *resource, wl_resource *modeResource)
{
    Q_Q(WlrOutputConfigurationHeadV1);

    if (modeChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "mode already set");
        return;
    }

    auto *outputMode = WlrOutputModeV1Private::fromResource(modeResource);

    // Mode can be nullptr if the head doesn't support modes, in which
    // case we'll expose a virtual mode based on current output size
    const auto modes = WlrOutputHeadV1Private::get(head)->modes;
    const bool found = (outputMode == nullptr && modes.size() == 0) || modes.contains(outputMode);
    if (!found) {
        wl_resource_post_error(resource->handle, error_invalid_mode,
                               "mode doesn't belong to head");
        return;
    }

    mode = outputMode;
    modeChanged = true;
    Q_EMIT q->modeChanged(mode);

    if (mode) {
        customModeSize = QSize(0, 0);
        customModeRefresh = 0;
        Q_EMIT q->customModeChanged(customModeSize, customModeRefresh);
    }
}

void WlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_custom_mode(Resource *resource, int32_t width, int32_t height, int32_t refresh)
{
    Q_UNUSED(resource)
    Q_Q(WlrOutputConfigurationHeadV1);

    if (width <= 0 || height <= 0 || refresh <= 0) {
        wl_resource_post_error(resource->handle, error_invalid_custom_mode,
                               "invalid custom mode");
        return;
    }

    if (customModeChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "custom mode already set");
        return;
    }

    mode = nullptr;
    Q_EMIT q->modeChanged(mode);

    customModeSize = QSize(width, height);
    customModeRefresh = refresh;
    customModeChanged = true;
    Q_EMIT q->customModeChanged(QSize(width, height), refresh);
}

void WlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_position(Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource)
    Q_Q(WlrOutputConfigurationHeadV1);

    if (positionChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "custom mode already set");
        return;
    }

    position = QPoint(x, y);
    positionChanged = true;
    Q_EMIT q->positionChanged(position);
}

void WlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_transform(Resource *resource, int32_t wlTransform)
{
    Q_Q(WlrOutputConfigurationHeadV1);

    if (transform < QWaylandOutput::TransformNormal ||
            transform > QWaylandOutput::TransformFlipped270) {
        wl_resource_post_error(resource->handle, error_invalid_transform,
                               "invalid transform");
        return;
    }

    if (transformChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "custom mode already set");
        return;
    }

    transform = static_cast<QWaylandOutput::Transform>(transform);
    transformChanged = true;
    Q_EMIT q->transformChanged(transform);
}

void WlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_scale(Resource *resource, wl_fixed_t scaleFixed)
{
    Q_Q(WlrOutputConfigurationHeadV1);

    qreal value = wl_fixed_to_double(scaleFixed);
    if (value <= 0) {
        wl_resource_post_error(resource->handle, error_invalid_scale,
                               "invalid scale");
        return;
    }

    if (scaleChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "custom mode already set");
        return;
    }

    scale = value;
    scaleChanged = true;
    Q_EMIT q->scaleChanged(scale);
}


WlrOutputConfigurationHeadV1::WlrOutputConfigurationHeadV1(WlrOutputHeadV1 *head, QObject *parent)
    : QObject(parent)
    , d_ptr(new WlrOutputConfigurationHeadV1Private(this))
{
    d_ptr->head = head;
}

WlrOutputConfigurationHeadV1::~WlrOutputConfigurationHeadV1()
{
    delete d_ptr;
}

WlrOutputHeadV1 *WlrOutputConfigurationHeadV1::head() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->head;
}

WlrOutputModeV1 *WlrOutputConfigurationHeadV1::mode() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->mode;
}

QSize WlrOutputConfigurationHeadV1::customModeSize() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->customModeSize;
}

qint32 WlrOutputConfigurationHeadV1::customModeRefresh() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->customModeRefresh;
}

QPoint WlrOutputConfigurationHeadV1::position() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->position;
}

QWaylandOutput::Transform WlrOutputConfigurationHeadV1::transform() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->transform;
}

qreal WlrOutputConfigurationHeadV1::scale() const
{
    Q_D(const WlrOutputConfigurationHeadV1);
    return d->scale;
}


WlrOutputConfigurationV1Private::WlrOutputConfigurationV1Private(WlrOutputConfigurationV1 *self)
    : QtWaylandServer::zwlr_output_configuration_v1()
    , q_ptr(self)
{
}

void WlrOutputConfigurationV1Private::zwlr_output_configuration_v1_enable_head(Resource *resource, uint32_t id, wl_resource *headResource)
{
    Q_Q(WlrOutputConfigurationV1);

    auto *head = WlrOutputHeadV1Private::fromResource(headResource);
    if (!head) {
        qCWarning(lcWaylandServer, "Invalid head resource");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_NO_MEMORY,
                               "invalid head resource");
        return;
    }

    if (configuredHeads.contains(head)) {
        wl_resource_post_error(resource->handle, error_already_configured_head,
                               "already configured head");
        return;
    }

    auto version = WlrOutputConfigurationHeadV1Private::interfaceVersion();
    auto *changes = new WlrOutputConfigurationHeadV1(head, q);
    WlrOutputConfigurationHeadV1Private::get(changes)->init(resource->client(), id, version);

    configuredHeads.append(head);
    enabledHeads.append(changes);

    Q_EMIT q->headEnabled(changes);
    Q_EMIT q->enabledHeadsChanged();
}

void WlrOutputConfigurationV1Private::zwlr_output_configuration_v1_disable_head(Resource *resource, wl_resource *headResource)
{
    Q_UNUSED(resource)
    Q_Q(WlrOutputConfigurationV1);

    auto *head = WlrOutputHeadV1Private::fromResource(headResource);
    if (!head) {
        qCWarning(lcWaylandServer, "Invalid head resource");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_NO_MEMORY,
                               "invalid head resource");
        return;
    }

    if (configuredHeads.contains(head)) {
        wl_resource_post_error(resource->handle, error_already_configured_head,
                               "already configured head");
        return;
    }

    configuredHeads.append(head);
    disabledHeads.append(head);

    Q_EMIT q->headDisabled(head);
    Q_EMIT q->disabledHeadsChanged();
}

void WlrOutputConfigurationV1Private::zwlr_output_configuration_v1_apply(Resource *resource)
{
    Q_Q(WlrOutputConfigurationV1);

    const auto values = manager->heads();
    for (auto *head : values) {
        if (!configuredHeads.contains(head)) {
            wl_resource_post_error(resource->handle, error_unconfigured_head,
                                   "unconfigured head %s", qPrintable(head->name()));
            return;
        }
    }

    Q_EMIT q->readyToApply();
}

void WlrOutputConfigurationV1Private::zwlr_output_configuration_v1_test(Resource *resource)
{
    Q_Q(WlrOutputConfigurationV1);

    const auto values = manager->heads();
    for (auto *head : values) {
        if (!configuredHeads.contains(head)) {
            wl_resource_post_error(resource->handle, error_unconfigured_head,
                                   "unconfigured head %s", qPrintable(head->name()));
            return;
        }
    }

    Q_EMIT q->readyToTest();
}

void WlrOutputConfigurationV1Private::zwlr_output_configuration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}


WlrOutputConfigurationV1::WlrOutputConfigurationV1(QObject *parent)
    : QObject(parent)
    , d_ptr(new WlrOutputConfigurationV1Private(this))
{
}

WlrOutputConfigurationV1::~WlrOutputConfigurationV1()
{
    delete d_ptr;
}

QVector<WlrOutputConfigurationHeadV1 *> WlrOutputConfigurationV1::enabledHeads() const
{
    Q_D(const WlrOutputConfigurationV1);
    return d->enabledHeads;
}

QVector<WlrOutputHeadV1 *> WlrOutputConfigurationV1::disabledHeads() const
{
    Q_D(const WlrOutputConfigurationV1);
    return d->disabledHeads;
}

#ifdef QT_WAYLAND_COMPOSITOR_QUICK
QQmlListProperty<WlrOutputConfigurationHeadV1> WlrOutputConfigurationV1::enabledHeadsList()
{
    auto countFunc = [](QQmlListProperty<WlrOutputConfigurationHeadV1> *prop) {
        return static_cast<WlrOutputConfigurationV1 *>(prop->object)->enabledHeads().size();
    };
    auto atFunc = [](QQmlListProperty<WlrOutputConfigurationHeadV1> *prop, int index) {
        return static_cast<WlrOutputConfigurationV1 *>(prop->object)->enabledHeads().at(index);
    };
    return QQmlListProperty<WlrOutputConfigurationHeadV1>(this, this, countFunc, atFunc);
}

QQmlListProperty<WlrOutputHeadV1> WlrOutputConfigurationV1::disabledHeadsList()
{
    auto countFunc = [](QQmlListProperty<WlrOutputHeadV1> *prop) {
        return static_cast<WlrOutputConfigurationV1 *>(prop->object)->disabledHeads().size();
    };
    auto atFunc = [](QQmlListProperty<WlrOutputHeadV1> *prop, int index) {
        return static_cast<WlrOutputConfigurationV1 *>(prop->object)->disabledHeads().at(index);
    };
    return QQmlListProperty<WlrOutputHeadV1>(this, this, countFunc, atFunc);
}
#endif

void WlrOutputConfigurationV1::sendSucceeded()
{
    Q_D(WlrOutputConfigurationV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_succeeded(resource->handle);
}

void WlrOutputConfigurationV1::sendFailed()
{
    Q_D(WlrOutputConfigurationV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_failed(resource->handle);
}

void WlrOutputConfigurationV1::sendCancelled()
{
    Q_D(WlrOutputConfigurationV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_cancelled(resource->handle);
}
