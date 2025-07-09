#include "MapWidget.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QApplication>
#include <QDebug>
#include <QtMath>

MapWidget::MapWidget(QWidget *parent)
    : QQuickWidget(parent)
    , m_latitude(DEFAULT_LATITUDE)
    , m_longitude(DEFAULT_LONGITUDE)
    , m_zoom(DEFAULT_ZOOM)
    , m_currentLayer("osm")
    , m_dragging(false)
    , m_updateTimer(new QTimer(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Setup update timer
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(50); // 20 FPS update rate
    connect(m_updateTimer, &QTimer::timeout, this, &MapWidget::updateCoordinates);
    
    setupQmlEngine();
    
    // Load the QML map view
    setSource(QUrl("qrc:/qt/qml/GeoWorld/MapView.qml"));
    
    // Connect QML signals
    connect(this, &QQuickWidget::statusChanged, [this](QQuickWidget::Status status) {
        if (status == QQuickWidget::Ready) {
            onMapReady();
        } else if (status == QQuickWidget::Error) {
            qWarning() << "QML Error:" << errors();
        }
    });
}

MapWidget::~MapWidget()
{
}

void MapWidget::setupQmlEngine()
{
    // Register this widget with QML context
    rootContext()->setContextProperty("mapWidget", this);
    
    // Set initial properties
    rootContext()->setContextProperty("initialLatitude", m_latitude);
    rootContext()->setContextProperty("initialLongitude", m_longitude);
    rootContext()->setContextProperty("initialZoom", m_zoom);
    rootContext()->setContextProperty("initialLayer", m_currentLayer);
}

void MapWidget::setMapLayer(const QString &layerType)
{
    if (m_currentLayer != layerType) {
        m_currentLayer = layerType;
        
        // Update QML property
        if (rootObject()) {
            QMetaObject::invokeMethod(rootObject(), "setMapLayer",
                                    Q_ARG(QVariant, layerType));
        }
    }
}

void MapWidget::resetView()
{
    setCenter(DEFAULT_LATITUDE, DEFAULT_LONGITUDE);
    setZoom(DEFAULT_ZOOM);
}

void MapWidget::zoomIn()
{
    setZoom(qMin(m_zoom + 1, MAX_ZOOM));
}

void MapWidget::zoomOut()
{
    setZoom(qMax(m_zoom - 1, MIN_ZOOM));
}

void MapWidget::setCenter(double latitude, double longitude)
{
    if (qAbs(m_latitude - latitude) > 0.000001 || qAbs(m_longitude - longitude) > 0.000001) {
        m_latitude = latitude;
        m_longitude = longitude;
        
        // Update QML map center
        if (rootObject()) {
            QMetaObject::invokeMethod(rootObject(), "setCenter",
                                    Q_ARG(QVariant, latitude),
                                    Q_ARG(QVariant, longitude));
        }
        
        emit coordinateChanged(m_latitude, m_longitude);
    }
}

void MapWidget::setZoom(int zoom)
{
    int clampedZoom = qBound(MIN_ZOOM, zoom, MAX_ZOOM);
    if (m_zoom != clampedZoom) {
        m_zoom = clampedZoom;
        
        // Update QML map zoom
        if (rootObject()) {
            QMetaObject::invokeMethod(rootObject(), "setZoom",
                                    Q_ARG(QVariant, m_zoom));
        }
        
        emit zoomChanged(m_zoom);
    }
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPanPoint = event->position();
        setCursor(Qt::ClosedHandCursor);
    }
    
    QQuickWidget::mousePressEvent(event);
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPointF delta = event->position() - m_lastPanPoint;
        m_lastPanPoint = event->position();
        
        // Convert pixel delta to coordinate delta (simplified calculation)
        double latDelta = -delta.y() * 0.001 * qPow(2, 8 - m_zoom);
        double lonDelta = -delta.x() * 0.001 * qPow(2, 8 - m_zoom);
        
        setCenter(m_latitude + latDelta, m_longitude + lonDelta);
    } else {
        // Update coordinate display for mouse position
        m_updateTimer->start();
    }
    
    QQuickWidget::mouseMoveEvent(event);
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        
        // Emit click signal for tools
        QPointF mapCoord = screenToMapCoordinate(event->position());
        emit mapClicked(mapCoord.y(), mapCoord.x()); // lat, lon
    }
    
    QQuickWidget::mouseReleaseEvent(event);
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
    
    QQuickWidget::wheelEvent(event);
}

void MapWidget::keyPressEvent(QKeyEvent *event)
{
    const double moveStep = 0.1 * qPow(2, 8 - m_zoom);
    
    switch (event->key()) {
    case Qt::Key_Up:
        setCenter(m_latitude + moveStep, m_longitude);
        break;
    case Qt::Key_Down:
        setCenter(m_latitude - moveStep, m_longitude);
        break;
    case Qt::Key_Left:
        setCenter(m_latitude, m_longitude - moveStep);
        break;
    case Qt::Key_Right:
        setCenter(m_latitude, m_longitude + moveStep);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Home:
        resetView();
        break;
    default:
        QQuickWidget::keyPressEvent(event);
        break;
    }
}

void MapWidget::onMapReady()
{
    qDebug() << "Map widget ready";
    updateMapProperties();
}

void MapWidget::onMapCenterChanged()
{
    // Handle center changes from QML
    if (rootObject()) {
        QVariant lat = rootObject()->property("latitude");
        QVariant lon = rootObject()->property("longitude");
        
        if (lat.isValid() && lon.isValid()) {
            m_latitude = lat.toDouble();
            m_longitude = lon.toDouble();
            emit coordinateChanged(m_latitude, m_longitude);
        }
    }
}

void MapWidget::onMapZoomChanged()
{
    // Handle zoom changes from QML
    if (rootObject()) {
        QVariant zoom = rootObject()->property("zoomLevel");
        if (zoom.isValid()) {
            m_zoom = zoom.toInt();
            emit zoomChanged(m_zoom);
        }
    }
}

void MapWidget::updateCoordinates()
{
    // Update coordinate display based on current mouse position
    QPointF mousePos = mapFromGlobal(QCursor::pos());
    if (rect().contains(mousePos.toPoint())) {
        QPointF mapCoord = screenToMapCoordinate(mousePos);
        emit coordinateChanged(mapCoord.y(), mapCoord.x()); // lat, lon
    }
}

void MapWidget::updateMapProperties()
{
    if (rootObject()) {
        rootObject()->setProperty("latitude", m_latitude);
        rootObject()->setProperty("longitude", m_longitude);
        rootObject()->setProperty("zoomLevel", m_zoom);
        rootObject()->setProperty("activeMapType", m_currentLayer);
    }
}

QPointF MapWidget::screenToMapCoordinate(const QPointF &screenPoint) const
{
    // Simplified Web Mercator projection conversion
    // This is a basic implementation - in production, use proper projection library
    
    double x = screenPoint.x() / width();
    double y = screenPoint.y() / height();
    
    // Convert to map coordinates (simplified)
    double lon = m_longitude + (x - 0.5) * 360.0 / qPow(2, m_zoom);
    double lat = m_latitude + (0.5 - y) * 180.0 / qPow(2, m_zoom);
    
    // Clamp to valid ranges
    lon = qBound(-180.0, lon, 180.0);
    lat = qBound(-85.0, lat, 85.0);
    
    return QPointF(lon, lat);
}

#include "MapWidget.moc"