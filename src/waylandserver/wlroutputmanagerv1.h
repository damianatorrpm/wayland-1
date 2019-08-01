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

#ifndef LIRI_WLROUTPUTMANAGERV1_H
#define LIRI_WLROUTPUTMANAGERV1_H

#include <QQmlComponent>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QWaylandCompositor>
#include <QWaylandCompositorExtension>
#include <QWaylandOutput>
#include <qwaylandquickchildren.h>

#include <LiriWaylandServer/liriwaylandserverglobal.h>

class QWaylandClient;

class WlrOutputConfigurationV1;
class WlrOutputConfigurationV1Private;
class WlrOutputConfigurationHeadV1;
class WlrOutputConfigurationHeadV1Private;
class WlrOutputManagerV1Private;
class WlrOutputHeadV1;
class WlrOutputHeadV1Private;
class WlrOutputModeV1;
class WlrOutputModeV1Private;

class LIRIWAYLANDSERVER_EXPORT WlrOutputManagerV1 : public QWaylandCompositorExtensionTemplate<WlrOutputManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WlrOutputManagerV1)
public:
    explicit WlrOutputManagerV1();
    WlrOutputManagerV1(QWaylandCompositor *compositor);
    ~WlrOutputManagerV1();

    void initialize() override;

    QWaylandCompositor *compositor() const;

    QVector<WlrOutputHeadV1 *> heads() const;

    Q_INVOKABLE void done(quint32 serial);
    Q_INVOKABLE void finished();

    static const wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void headAdded(WlrOutputHeadV1 *head);
    void configurationCreated(WlrOutputConfigurationV1 *configuration);
    void clientStopped(QWaylandClient *client);

private:
    WlrOutputManagerV1Private *const d_ptr;
};

class LIRIWAYLANDSERVER_EXPORT WlrOutputHeadV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WlrOutputHeadV1)
    Q_PROPERTY(WlrOutputManagerV1 *manager READ manager WRITE setManager NOTIFY managerChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QSize physicalSize READ physicalSize WRITE setPhysicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(QPoint position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(WlrOutputModeV1 *currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(WlrOutputModeV1 *preferredMode READ preferredMode WRITE setPreferredMode NOTIFY preferredModeChanged)
    Q_PROPERTY(QWaylandOutput::Transform transform READ transform WRITE setTransform NOTIFY transformChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
public:
    explicit WlrOutputHeadV1(QObject *parent = nullptr);
    ~WlrOutputHeadV1();

    bool isInitialized() const;
    void initialize();

    WlrOutputManagerV1 *manager() const;
    void setManager(WlrOutputManagerV1 *manager);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    QSize physicalSize() const;
    void setPhysicalSize(const QSize &physicalSize);

    QPoint position() const;
    void setPosition(const QPoint &position);

    QVector<WlrOutputModeV1 *> modes() const;
    Q_INVOKABLE void addMode(WlrOutputModeV1 *mode);

    WlrOutputModeV1 *currentMode() const;
    void setCurrentMode(WlrOutputModeV1 *mode);

    WlrOutputModeV1 *preferredMode() const;
    void setPreferredMode(WlrOutputModeV1 *mode);

    QWaylandOutput::Transform transform() const;
    void setTransform(QWaylandOutput::Transform transform);

    qreal scale() const;
    void setScale(qreal scale);

Q_SIGNALS:
    void managerChanged();
    void enabledChanged();
    void nameChanged();
    void descriptionChanged();
    void physicalSizeChanged();
    void positionChanged();
    void modeAdded(WlrOutputModeV1 *mode);
    void modesChanged();
    void currentModeChanged();
    void preferredModeChanged();
    void transformChanged();
    void scaleChanged();

private:
    WlrOutputHeadV1Private *const d_ptr;
};

QML_DECLARE_TYPE(WlrOutputHeadV1)

class LIRIWAYLANDSERVER_EXPORT WlrOutputHeadV1Qml : public WlrOutputHeadV1, public QQmlParserStatus
{
    Q_OBJECT
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(WlrOutputHeadV1Qml)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<WlrOutputModeV1> modes READ modesList NOTIFY modesChanged)
public:
    explicit WlrOutputHeadV1Qml(QObject *parent = nullptr);

    QQmlListProperty<WlrOutputModeV1> modesList();

protected:
    void classBegin() override {}
    void componentComplete() override;
};

QML_DECLARE_TYPE(WlrOutputHeadV1Qml)

class LIRIWAYLANDSERVER_EXPORT WlrOutputModeV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WlrOutputModeV1)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(WlrOutputModeV1)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(qint32 refresh READ refresh WRITE setRefresh NOTIFY refreshChanged)
public:
    explicit WlrOutputModeV1(QObject *parent = nullptr);

    bool isInitialized() const;

    QSize size() const;
    void setSize(const QSize &size);

    qint32 refresh() const;
    void setRefresh(qint32 refresh);

Q_SIGNALS:
    void sizeChanged();
    void refreshChanged();

private:
    WlrOutputModeV1Private *const d_ptr;
};

QML_DECLARE_TYPE(WlrOutputModeV1)

class LIRIWAYLANDSERVER_EXPORT WlrOutputConfigurationHeadV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WlrOutputConfigurationHeadV1)
    Q_PROPERTY(WlrOutputHeadV1 *head READ head CONSTANT)
    Q_PROPERTY(WlrOutputModeV1 *mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QSize customModeSize READ customModeSize NOTIFY customModeChanged)
    Q_PROPERTY(qint32 customModeRefresh READ customModeRefresh NOTIFY customModeChanged)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(QWaylandOutput::Transform transform READ transform NOTIFY transformChanged)
    Q_PROPERTY(qreal scale READ scale NOTIFY scaleChanged)
public:
    ~WlrOutputConfigurationHeadV1();

    WlrOutputHeadV1 *head() const;
    WlrOutputModeV1 *mode() const;
    QSize customModeSize() const;
    qint32 customModeRefresh() const;
    QPoint position() const;
    QWaylandOutput::Transform transform() const;
    qreal scale() const;

Q_SIGNALS:
    void modeChanged(WlrOutputModeV1 *mode);
    void customModeChanged(const QSize &size, qint32 refreshRate);
    void positionChanged(const QPoint &position);
    void transformChanged(QWaylandOutput::Transform transform);
    void scaleChanged(qreal scale);

private:
    WlrOutputConfigurationHeadV1Private *const d_ptr;

    explicit WlrOutputConfigurationHeadV1(WlrOutputHeadV1 *head, QObject *parent = nullptr);

    friend class WlrOutputConfigurationV1Private;
};

QML_DECLARE_TYPE(WlrOutputConfigurationHeadV1)

class LIRIWAYLANDSERVER_EXPORT WlrOutputConfigurationV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WlrOutputConfigurationV1)
#ifdef QT_WAYLAND_COMPOSITOR_QUICK
    Q_PROPERTY(QQmlListProperty<WlrOutputConfigurationHeadV1> enabledHeads READ enabledHeadsList NOTIFY enabledHeadsChanged)
    Q_PROPERTY(QQmlListProperty<WlrOutputHeadV1> disabledHeads READ disabledHeadsList NOTIFY disabledHeadsChanged)
#endif
public:
    explicit WlrOutputConfigurationV1(QObject *parent = nullptr);
    ~WlrOutputConfigurationV1();

    QVector<WlrOutputConfigurationHeadV1 *> enabledHeads() const;
    QVector<WlrOutputHeadV1 *> disabledHeads() const;

#ifdef QT_WAYLAND_COMPOSITOR_QUICK
    QQmlListProperty<WlrOutputConfigurationHeadV1> enabledHeadsList();
    QQmlListProperty<WlrOutputHeadV1> disabledHeadsList();
#endif

    Q_INVOKABLE void sendSucceeded();
    Q_INVOKABLE void sendFailed();
    Q_INVOKABLE void sendCancelled();

Q_SIGNALS:
    void enabledHeadsChanged();
    void disabledHeadsChanged();
    void headEnabled(WlrOutputConfigurationHeadV1 *headChanges);
    void headDisabled(WlrOutputHeadV1 *head);
    void readyToApply();
    void readyToTest();

private:
    WlrOutputConfigurationV1Private *const d_ptr;
};

#endif // LIRI_WLROUTPUTMANAGERV1_H
