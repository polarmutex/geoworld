#include "MapWidget.h"
#include <QApplication>
#include <QDebug>
#include <QtMath>
#include <QPainter>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QUrl>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , m_latitude(DEFAULT_LATITUDE)
    , m_longitude(DEFAULT_LONGITUDE)
    , m_zoom(DEFAULT_ZOOM)
    , m_currentLayer("osm")
    , m_dragging(false)
    , m_updateTimer(new QTimer(this))
    , m_networkManager(new QNetworkAccessManager(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(400, 300);
    
    // Setup update timer
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(50); // 20 FPS update rate
    connect(m_updateTimer, &QTimer::timeout, this, &MapWidget::updateCoordinates);
    
    setupMap();
}

MapWidget::~MapWidget()
{
}

void MapWidget::setupMap()
{
    // Initialize map display
    m_mapPixmap = QPixmap(512, 512);
    m_mapPixmap.fill(QColor(200, 230, 255)); // Light blue background
    
    // Draw a simple placeholder map
    QPainter painter(&m_mapPixmap);
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(QBrush(Qt::lightGray));
    
    // Draw some mock continents
    painter.drawEllipse(50, 50, 100, 80);   // Mock continent 1
    painter.drawEllipse(200, 150, 150, 100); // Mock continent 2
    painter.drawEllipse(300, 300, 120, 90);  // Mock continent 3
    
    // Draw grid lines
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    for (int i = 0; i < 512; i += 64) {
        painter.drawLine(i, 0, i, 512);
        painter.drawLine(0, i, 512, i);
    }
    
    // Draw center crosshair
    painter.setPen(QPen(Qt::red, 2));
    painter.drawLine(252, 256, 260, 256);
    painter.drawLine(256, 252, 256, 260);
    
    update();
}

void MapWidget::updateMapDisplay()
{
    // Just trigger a repaint - we'll draw directly in paintEvent
    update();
}

void MapWidget::loadMapTile(int x, int y, int zoom)
{
    // Placeholder for tile loading - in a real implementation,
    // this would download OpenStreetMap tiles
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(zoom)
}

void MapWidget::onTileDownloaded()
{
    // Placeholder for tile download completion
    update();
}

void MapWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    // Fill background
    painter.fillRect(rect(), QColor(200, 230, 255));
    
    // Calculate offset based on current coordinates
    int centerX = width() / 2;
    int centerY = height() / 2;
    int offsetX = (int)(m_longitude * 2); // Simple scaling for demo
    int offsetY = (int)(m_latitude * 2);
    
    // Draw mock continents relative to current position
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(QBrush(Qt::lightGray));
    
    painter.drawEllipse(centerX + 50 - offsetX, centerY + 50 - offsetY, 100, 80);
    painter.drawEllipse(centerX + 200 - offsetX, centerY + 150 - offsetY, 150, 100);
    painter.drawEllipse(centerX + 300 - offsetX, centerY + 300 - offsetY, 120, 90);
    
    // Draw grid lines
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    for (int i = -offsetX % 64; i < width(); i += 64) {
        if (i >= 0) painter.drawLine(i, 0, i, height());
    }
    for (int i = -offsetY % 64; i < height(); i += 64) {
        if (i >= 0) painter.drawLine(0, i, width(), i);
    }
    
    // Draw center crosshair
    painter.setPen(QPen(Qt::red, 2));
    painter.drawLine(centerX - 8, centerY, centerX + 8, centerY);
    painter.drawLine(centerX, centerY - 8, centerX, centerY + 8);
    
    // Draw coordinate information
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10));
    
    QString coordText = QString("Lat: %1, Lon: %2, Zoom: %3")
                        .arg(m_latitude, 0, 'f', 3)
                        .arg(m_longitude, 0, 'f', 3)
                        .arg(m_zoom);
    
    painter.drawText(10, 20, coordText);
    
    // Draw layer info
    painter.drawText(10, 40, QString("Layer: %1").arg(m_currentLayer));
    
    // Draw scale bar
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    QRect scaleRect(10, height() - 40, 100, 20);
    painter.drawRect(scaleRect);
    painter.drawText(scaleRect, Qt::AlignCenter, "Scale: 1km");
    
    // Draw zoom controls
    QRect zoomInRect(width() - 40, 10, 30, 30);
    QRect zoomOutRect(width() - 40, 45, 30, 30);
    
    painter.setBrush(Qt::lightGray);
    painter.drawRect(zoomInRect);
    painter.drawRect(zoomOutRect);
    
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(zoomInRect, Qt::AlignCenter, "+");
    painter.drawText(zoomOutRect, Qt::AlignCenter, "-");
}

void MapWidget::setMapLayer(const QString &layerType)
{
    m_currentLayer = layerType;
    updateMapDisplay();
}

void MapWidget::setCenter(double latitude, double longitude)
{
    m_latitude = latitude;
    m_longitude = longitude;
    updateMapDisplay();
    
    emit coordinateChanged(m_latitude, m_longitude);
}

void MapWidget::setZoom(int zoom)
{
    m_zoom = qBound(1, zoom, 18);
    updateMapDisplay();
    
    emit zoomChanged(m_zoom);
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if clicking on zoom controls
        QRect zoomInRect(width() - 40, 10, 30, 30);
        QRect zoomOutRect(width() - 40, 45, 30, 30);
        
        if (zoomInRect.contains(event->pos())) {
            setZoom(m_zoom + 1);
            return;
        } else if (zoomOutRect.contains(event->pos())) {
            setZoom(m_zoom - 1);
            return;
        }
        
        m_dragging = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        setMouseTracking(true);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPointF delta = event->pos() - m_lastPanPoint;
        
        // Simple pixel to coordinate conversion
        double latDelta = delta.y() * 0.01;
        double lonDelta = -delta.x() * 0.01;
        
        m_latitude += latDelta;
        m_longitude += lonDelta;
        
        // Clamp coordinates
        m_latitude = qBound(-90.0, m_latitude, 90.0);
        m_longitude = qBound(-180.0, m_longitude, 180.0);
        
        m_lastPanPoint = event->pos();
        
        // Immediately regenerate the map
        updateMapDisplay();
        
        emit coordinateChanged(m_latitude, m_longitude);
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        setMouseTracking(false);
    }
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    
    setZoom(m_zoom + numSteps);
}

void MapWidget::keyPressEvent(QKeyEvent *event)
{
    const double moveStep = 0.01;
    
    switch (event->key()) {
        case Qt::Key_Left:
            setCenter(m_latitude, m_longitude - moveStep);
            break;
        case Qt::Key_Right:
            setCenter(m_latitude, m_longitude + moveStep);
            break;
        case Qt::Key_Up:
            setCenter(m_latitude + moveStep, m_longitude);
            break;
        case Qt::Key_Down:
            setCenter(m_latitude - moveStep, m_longitude);
            break;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            setZoom(m_zoom + 1);
            break;
        case Qt::Key_Minus:
            setZoom(m_zoom - 1);
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

void MapWidget::updateCoordinates()
{
    // This would normally update the map display based on current coordinates
    update();
}

QPointF MapWidget::screenToMapCoordinate(const QPointF &screenPoint) const
{
    // Simple screen to map coordinate conversion
    double x = (screenPoint.x() / width()) * 360.0 - 180.0;
    double y = 90.0 - (screenPoint.y() / height()) * 180.0;
    return QPointF(x, y);
}

// Public slot implementations for Qt's MOC system
void MapWidget::resetView()
{
    setCenter(DEFAULT_LATITUDE, DEFAULT_LONGITUDE);
    setZoom(DEFAULT_ZOOM);
}

void MapWidget::zoomIn()
{
    setZoom(m_zoom + 1);
}

void MapWidget::zoomOut()
{
    setZoom(m_zoom - 1);
}