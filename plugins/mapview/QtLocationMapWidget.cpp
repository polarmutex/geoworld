#include "QtLocationMapWidget.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QtMath>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>

QtLocationMapWidget::QtLocationMapWidget(QWidget *parent)
    : QWidget(parent)
    , m_latitude(DEFAULT_LATITUDE)
    , m_longitude(DEFAULT_LONGITUDE)
    , m_zoom(DEFAULT_ZOOM)
    , m_currentLayer("osm")
    , m_dragging(false)
    , m_mapOffset(0, 0)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateTimer(new QTimer(this))
    , m_positionSource(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(600, 400);
    
    setupUI();
    setupLocationService();
    
    // Setup update timer
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(100);
    connect(m_updateTimer, &QTimer::timeout, this, &QtLocationMapWidget::updateMapDisplay);
    
    // Load initial tiles
    loadVisibleTiles();
}

QtLocationMapWidget::~QtLocationMapWidget()
{
    if (m_positionSource) {
        m_positionSource->stopUpdates();
    }
}

void QtLocationMapWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // Controls layout at top
    m_controlsLayout = new QHBoxLayout();
    m_controlsLayout->setContentsMargins(5, 5, 5, 5);
    
    // Layer selection
    m_layerCombo = new QComboBox();
    m_layerCombo->addItem("OpenStreetMap", "osm");
    m_layerCombo->addItem("OpenStreetMap Humanitarian", "osm-humanitarian");
    m_layerCombo->setCurrentIndex(0);
    m_layerCombo->setMaximumWidth(200);
    connect(m_layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) {
                setMapLayer(m_layerCombo->itemData(index).toString());
            });
    
    // Zoom controls
    m_zoomOutButton = new QPushButton("-");
    m_zoomOutButton->setMaximumWidth(30);
    m_zoomInButton = new QPushButton("+");
    m_zoomInButton->setMaximumWidth(30);
    
    connect(m_zoomInButton, &QPushButton::clicked, this, &QtLocationMapWidget::zoomIn);
    connect(m_zoomOutButton, &QPushButton::clicked, this, &QtLocationMapWidget::zoomOut);
    
    // Zoom slider
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(MIN_ZOOM, MAX_ZOOM);
    m_zoomSlider->setValue(m_zoom);
    m_zoomSlider->setMaximumWidth(150);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &QtLocationMapWidget::setZoom);
    
    // Reset and locate buttons
    m_resetButton = new QPushButton("Reset");
    m_resetButton->setMaximumWidth(60);
    m_locateButton = new QPushButton("Locate");
    m_locateButton->setMaximumWidth(60);
    
    connect(m_resetButton, &QPushButton::clicked, this, &QtLocationMapWidget::resetView);
    connect(m_locateButton, &QPushButton::clicked, [this]() {
        if (m_positionSource) {
            m_positionSource->requestUpdate();
        }
    });
    
    // Coordinate label
    m_coordLabel = new QLabel();
    m_coordLabel->setMinimumWidth(200);
    
    // Add controls to layout
    m_controlsLayout->addWidget(new QLabel("Layer:"));
    m_controlsLayout->addWidget(m_layerCombo);
    m_controlsLayout->addSpacing(10);
    m_controlsLayout->addWidget(new QLabel("Zoom:"));
    m_controlsLayout->addWidget(m_zoomOutButton);
    m_controlsLayout->addWidget(m_zoomSlider);
    m_controlsLayout->addWidget(m_zoomInButton);
    m_controlsLayout->addSpacing(10);
    m_controlsLayout->addWidget(m_resetButton);
    m_controlsLayout->addWidget(m_locateButton);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(m_coordLabel);
    
    // Create the map area widget (this will be where we paint the map)
    m_mapArea = new QWidget();
    m_mapArea->setMinimumSize(400, 300);
    m_mapArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Add to main layout
    m_layout->addLayout(m_controlsLayout);
    m_layout->addWidget(m_mapArea, 1); // Give map area most of the space
    
    // Update coordinate display
    updateMapDisplay();
}

void QtLocationMapWidget::setupLocationService()
{
    m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    if (m_positionSource) {
        connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated,
                this, &QtLocationMapWidget::onPositionUpdated);
        m_positionSource->setUpdateInterval(5000); // 5 second updates
    }
}

void QtLocationMapWidget::loadVisibleTiles()
{
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start();
    }
}

void QtLocationMapWidget::loadTile(int x, int y, int z)
{
    QString cacheKey = getTileCacheKey(x, y, z);
    
    if (m_tileCache.contains(cacheKey) && m_tileCache[cacheKey].loaded) {
        return; // Already loaded
    }
    
    // Check file cache first
    QString cachePath = getTileCachePath(x, y, z);
    if (QFile::exists(cachePath)) {
        QPixmap pixmap(cachePath);
        if (!pixmap.isNull()) {
            TileInfo tile;
            tile.x = x;
            tile.y = y;
            tile.z = z;
            tile.pixmap = pixmap;
            tile.loaded = true;
            m_tileCache[cacheKey] = tile;
            update();
            return;
        }
    }
    
    // Download tile
    QString url = getTileUrl(x, y, z);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "GeoWorld/1.0");
    
    QNetworkReply *reply = m_networkManager->get(request);
    reply->setProperty("x", x);
    reply->setProperty("y", y);
    reply->setProperty("z", z);
    
    connect(reply, &QNetworkReply::finished, this, &QtLocationMapWidget::onTileDownloaded);
}

QString QtLocationMapWidget::getTileUrl(int x, int y, int z) const
{
    if (m_currentLayer == "osm-humanitarian") {
        return QString("https://tile-{s}.openstreetmap.fr/hot/%1/%2/%3.png")
                .arg(z).arg(x).arg(y);
    } else {
        // Default to OpenStreetMap
        return QString("https://tile.openstreetmap.org/%1/%2/%3.png")
                .arg(z).arg(x).arg(y);
    }
}

QString QtLocationMapWidget::getTileCacheKey(int x, int y, int z) const
{
    return QString("%1_%2_%3_%4").arg(m_currentLayer).arg(z).arg(x).arg(y);
}

QString QtLocationMapWidget::getTileCachePath(int x, int y, int z) const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    cacheDir += "/tiles/" + m_currentLayer;
    QDir().mkpath(cacheDir);
    return QString("%1/%2_%3_%4.png").arg(cacheDir).arg(z).arg(x).arg(y);
}

void QtLocationMapWidget::onTileDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    int x = reply->property("x").toInt();
    int y = reply->property("y").toInt();
    int z = reply->property("z").toInt();
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        if (pixmap.loadFromData(data)) {
            // Save to cache
            QString cachePath = getTileCachePath(x, y, z);
            pixmap.save(cachePath);
            
            // Add to memory cache
            TileInfo tile;
            tile.x = x;
            tile.y = y;
            tile.z = z;
            tile.pixmap = pixmap;
            tile.loaded = true;
            
            QString cacheKey = getTileCacheKey(x, y, z);
            m_tileCache[cacheKey] = tile;
            
            update();
        }
    }
    
    reply->deleteLater();
}

void QtLocationMapWidget::onPositionUpdated(const QGeoPositionInfo &info)
{
    if (info.isValid()) {
        QGeoCoordinate coord = info.coordinate();
        setCenter(coord.latitude(), coord.longitude());
    }
}

void QtLocationMapWidget::updateMapDisplay()
{
    // Update coordinate display
    if (m_coordLabel) {
        m_coordLabel->setText(QString("Lat: %1, Lon: %2, Zoom: %3")
                              .arg(m_latitude, 0, 'f', 6)
                              .arg(m_longitude, 0, 'f', 6)
                              .arg(m_zoom));
    }
    
    // Update zoom slider
    if (m_zoomSlider) {
        m_zoomSlider->setValue(m_zoom);
    }
    
    // Load visible tiles
    if (m_mapArea) {
        QPoint centerPixel = latLonToPixel(m_latitude, m_longitude, m_zoom);
        int tilesX = (m_mapArea->width() / TILE_SIZE) + 2;
        int tilesY = (m_mapArea->height() / TILE_SIZE) + 2;
        
        int startX = (centerPixel.x() / TILE_SIZE) - (tilesX / 2);
        int startY = (centerPixel.y() / TILE_SIZE) - (tilesY / 2);
        
        for (int x = startX; x < startX + tilesX; ++x) {
            for (int y = startY; y < startY + tilesY; ++y) {
                if (x >= 0 && y >= 0 && x < (1 << m_zoom) && y < (1 << m_zoom)) {
                    loadTile(x, y, m_zoom);
                }
            }
        }
    }
    
    update();
}

QPoint QtLocationMapWidget::latLonToPixel(double lat, double lon, int zoom) const
{
    double scale = 1 << zoom;
    double x = (lon + 180.0) / 360.0 * scale * TILE_SIZE;
    double y = (1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * scale * TILE_SIZE;
    return QPoint((int)x, (int)y);
}

QGeoCoordinate QtLocationMapWidget::pixelToLatLon(const QPoint &pixel, int zoom) const
{
    double scale = 1 << zoom;
    double lon = pixel.x() / (scale * TILE_SIZE) * 360.0 - 180.0;
    double n = M_PI - 2.0 * M_PI * pixel.y() / (scale * TILE_SIZE);
    double lat = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
    return QGeoCoordinate(lat, lon);
}

void QtLocationMapWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    if (!m_mapArea) return;
    
    QPainter painter(this);
    
    // Get map area position relative to this widget
    QRect mapRect = m_mapArea->geometry();
    
    // Fill map background
    painter.fillRect(mapRect, QColor(200, 230, 255));
    
    // Calculate map center pixel
    QPoint centerPixel = latLonToPixel(m_latitude, m_longitude, m_zoom);
    QPoint mapCenter(mapRect.center());
    QPoint offset = mapCenter - centerPixel + m_mapOffset;
    
    // Draw tiles within map area
    painter.setClipRect(mapRect);
    for (auto it = m_tileCache.begin(); it != m_tileCache.end(); ++it) {
        const TileInfo &tile = it.value();
        if (tile.loaded && tile.z == m_zoom) {
            QPoint tilePixel(tile.x * TILE_SIZE, tile.y * TILE_SIZE);
            QPoint drawPos = tilePixel + offset;
            
            // Only draw if visible in map area
            QRect tileRect(drawPos, QSize(TILE_SIZE, TILE_SIZE));
            if (tileRect.intersects(mapRect)) {
                painter.drawPixmap(drawPos, tile.pixmap);
            }
        }
    }
    
    // Draw center crosshair
    painter.setPen(QPen(Qt::red, 2));
    painter.drawLine(mapCenter.x() - 10, mapCenter.y(), mapCenter.x() + 10, mapCenter.y());
    painter.drawLine(mapCenter.x(), mapCenter.y() - 10, mapCenter.x(), mapCenter.y() + 10);
}

void QtLocationMapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    loadVisibleTiles();
}

void QtLocationMapWidget::setMapLayer(const QString &layerType)
{
    m_currentLayer = layerType;
    m_tileCache.clear();
    loadVisibleTiles();
}

void QtLocationMapWidget::resetView()
{
    setCenter(DEFAULT_LATITUDE, DEFAULT_LONGITUDE);
    setZoom(DEFAULT_ZOOM);
}

void QtLocationMapWidget::zoomIn()
{
    setZoom(m_zoom + 1);
}

void QtLocationMapWidget::zoomOut()
{
    setZoom(m_zoom - 1);
}

void QtLocationMapWidget::setCenter(double latitude, double longitude)
{
    m_latitude = qBound(-85.0, latitude, 85.0);
    m_longitude = qBound(-180.0, longitude, 180.0);
    
    loadVisibleTiles();
    
    emit coordinateChanged(m_latitude, m_longitude);
}

void QtLocationMapWidget::setZoom(int zoom)
{
    int newZoom = qBound(MIN_ZOOM, zoom, MAX_ZOOM);
    if (newZoom != m_zoom) {
        m_zoom = newZoom;
        m_tileCache.clear();
        loadVisibleTiles();
        emit zoomChanged(m_zoom);
    }
}

void QtLocationMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_mapArea) {
        // Check if click is within map area
        if (m_mapArea->geometry().contains(event->pos())) {
            m_dragging = true;
            m_lastPanPoint = event->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }
}

void QtLocationMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_mapOffset += delta;
        m_lastPanPoint = event->pos();
        update();
    }
}

void QtLocationMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        
        // Convert map offset to coordinate change
        QPoint centerPixel = latLonToPixel(m_latitude, m_longitude, m_zoom);
        QPoint newCenterPixel = centerPixel - m_mapOffset;
        QGeoCoordinate newCenter = pixelToLatLon(newCenterPixel, m_zoom);
        
        m_mapOffset = QPoint(0, 0);
        setCenter(newCenter.latitude(), newCenter.longitude());
        
        emit mapClicked(newCenter.latitude(), newCenter.longitude());
    }
}

void QtLocationMapWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    
    setZoom(m_zoom + numSteps);
}

void QtLocationMapWidget::keyPressEvent(QKeyEvent *event)
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
            zoomIn();
            break;
        case Qt::Key_Minus:
            zoomOut();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}