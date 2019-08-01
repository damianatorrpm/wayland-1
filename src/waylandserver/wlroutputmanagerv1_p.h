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

#ifndef LIRI_WLROUTPUTMANAGERV1_P_H
#define LIRI_WLROUTPUTMANAGERV1_P_H

#include <QPoint>
#include <QPointer>
#include <QSize>

#include <LiriWaylandServer/WlrOutputManagerV1>
#include <LiriWaylandServer/private/qwayland-server-wlr-output-management-unstable-v1.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Liri API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class LIRIWAYLANDSERVER_EXPORT WlrOutputManagerV1Private
        : public QtWaylandServer::zwlr_output_manager_v1
{
    Q_DECLARE_PUBLIC(WlrOutputManagerV1)
public:
    explicit WlrOutputManagerV1Private(WlrOutputManagerV1 *self);
    ~WlrOutputManagerV1Private();

    void registerHead(WlrOutputHeadV1 *head);

    static WlrOutputManagerV1Private *get(WlrOutputManagerV1 *manager) { return manager->d_func(); }

    QWaylandCompositor *compositor = nullptr;
    QVector<WlrOutputHeadV1 *> heads;
    QVector<wl_client *> stoppedClients;
    QMap<quint32, WlrOutputConfigurationV1 *> configurations;

protected:
    WlrOutputManagerV1 *q_ptr;

    void zwlr_output_manager_v1_bind_resource(Resource *resource) override;
    void zwlr_output_manager_v1_create_configuration(Resource *resource, uint32_t id, uint32_t serial) override;
    void zwlr_output_manager_v1_stop(Resource *resource) override;
};

class LIRIWAYLANDSERVER_EXPORT WlrOutputHeadV1Private
        : public QtWaylandServer::zwlr_output_head_v1
{
    Q_DECLARE_PUBLIC(WlrOutputHeadV1)
public:
    explicit WlrOutputHeadV1Private(WlrOutputHeadV1 *self);
    ~WlrOutputHeadV1Private();

    void sendInfo(Resource *resource);

    static WlrOutputHeadV1 *fromResource(wl_resource *resource);

    static WlrOutputHeadV1Private *get(WlrOutputHeadV1 *head) { return head->d_func(); }

    WlrOutputManagerV1 *manager = nullptr;
    bool initialized = false;
    bool modesSent = false;
    QString name;
    bool nameChanged = false;
    QString description;
    bool descriptionChanged = false;
    QSize physicalSize;
    QVector<WlrOutputModeV1 *> modes;
    QPointer<WlrOutputModeV1> currentMode;
    QPointer<WlrOutputModeV1> preferredMode;
    bool enabled = true;
    QPoint position;
    QWaylandOutput::Transform transform = QWaylandOutput::TransformNormal;
    qreal scale = 1;

protected:
    WlrOutputHeadV1 *q_ptr;
};

class LIRIWAYLANDSERVER_EXPORT WlrOutputModeV1Private
        : public QtWaylandServer::zwlr_output_mode_v1
{
    Q_DECLARE_PUBLIC(WlrOutputModeV1)
public:
    explicit WlrOutputModeV1Private(WlrOutputModeV1 *self);
    ~WlrOutputModeV1Private();

    static WlrOutputModeV1 *fromResource(wl_resource *resource);

    static WlrOutputModeV1Private *get(WlrOutputModeV1 *mode) { return mode->d_func(); }

    bool initialized = false;
    QSize size;
    qint32 refreshRate = -1;

protected:
    WlrOutputModeV1 *q_ptr;
};

class LIRIWAYLANDSERVER_EXPORT WlrOutputConfigurationV1Private
        : public QtWaylandServer::zwlr_output_configuration_v1
{
    Q_DECLARE_PUBLIC(WlrOutputConfigurationV1)
public:
    explicit WlrOutputConfigurationV1Private(WlrOutputConfigurationV1 *self);

    static WlrOutputConfigurationV1Private *get(WlrOutputConfigurationV1 *configuration) { return configuration->d_func(); }

    WlrOutputManagerV1 *manager = nullptr;
    QVector<WlrOutputConfigurationHeadV1 *> enabledHeads;
    QVector<WlrOutputHeadV1 *> disabledHeads;
    QVector<WlrOutputHeadV1 *> configuredHeads;

protected:
    WlrOutputConfigurationV1 *q_ptr;

    void zwlr_output_configuration_v1_enable_head(Resource *resource, uint32_t id, struct ::wl_resource *headResource) override;
    void zwlr_output_configuration_v1_disable_head(Resource *resource, struct ::wl_resource *headResource) override;
    void zwlr_output_configuration_v1_apply(Resource *resource) override;
    void zwlr_output_configuration_v1_test(Resource *resource) override;
    void zwlr_output_configuration_v1_destroy(Resource *resource) override;
};

class LIRIWAYLANDSERVER_EXPORT WlrOutputConfigurationHeadV1Private
        : public QtWaylandServer::zwlr_output_configuration_head_v1
{
    Q_DECLARE_PUBLIC(WlrOutputConfigurationHeadV1)
public:
    explicit WlrOutputConfigurationHeadV1Private(WlrOutputConfigurationHeadV1 *self);

    static WlrOutputConfigurationHeadV1Private *get(WlrOutputConfigurationHeadV1 *configuration) { return configuration->d_func(); }

    WlrOutputHeadV1 *head = nullptr;
    WlrOutputModeV1 *mode = nullptr;
    QSize customModeSize = QSize(0, 0);
    qint32 customModeRefresh = 0;
    QPoint position;
    QWaylandOutput::Transform transform = QWaylandOutput::TransformNormal;
    qreal scale = 1;

    bool modeChanged = false;
    bool customModeChanged = false;
    bool positionChanged = false;
    bool transformChanged = false;
    bool scaleChanged = false;

protected:
    WlrOutputConfigurationHeadV1 *q_ptr;

    void zwlr_output_configuration_head_v1_set_mode(Resource *resource, struct ::wl_resource *modeResource) override;
    void zwlr_output_configuration_head_v1_set_custom_mode(Resource *resource, int32_t width, int32_t height, int32_t refresh) override;
    void zwlr_output_configuration_head_v1_set_position(Resource *resource, int32_t x, int32_t y) override;
    void zwlr_output_configuration_head_v1_set_transform(Resource *resource, int32_t wlTransform) override;
    void zwlr_output_configuration_head_v1_set_scale(Resource *resource, wl_fixed_t scaleFixed) override;
};

#endif // LIRI_WLROUTPUTMANAGERV1_P_H
