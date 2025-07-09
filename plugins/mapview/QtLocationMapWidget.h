#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QScrollArea>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QGeoServiceProvider>
#include <QHash>
#include <QDir>
#include <QStandardPaths>

class QtLocationMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QtLocationMapWidget(QWidget *parent = nullptr);
    ~QtLocationMapWidget();

    void setMapLayer(const QString &layerType);
    void resetView();
    void zoomIn();
    void zoomOut();
    void setCenter(double latitude, double longitude);
    void setZoom(int zoom);

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    int zoom() const { return m_zoom; }

signals:
    void coordinateChanged(double latitude, double longitude);
    void zoomChanged(int zoom);
    void mapClicked(double latitude, double longitude);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTileDownloaded();
    void onPositionUpdated(const QGeoPositionInfo &info);
    void updateMapDisplay();

private:
    struct TileInfo {
        int x, y, z;
        QPixmap pixmap;
        bool loaded;
    };

    void setupUI();
    void setupLocationService();
    void loadVisibleTiles();
    void loadTile(int x, int y, int z);
    QString getTileUrl(int x, int y, int z) const;
    QString getTileCacheKey(int x, int y, int z) const;
    QString getTileCachePath(int x, int y, int z) const;
    QPoint latLonToPixel(double lat, double lon, int zoom) const;
    QGeoCoordinate pixelToLatLon(const QPoint &pixel, int zoom) const;
    void drawTile(QPainter &painter, const TileInfo &tile);
    void drawControls(QPainter &painter);
    void drawCoordinateInfo(QPainter &painter);

    // Map state
    double m_latitude;
    double m_longitude;
    int m_zoom;
    QString m_currentLayer;
    
    // Interaction state
    bool m_dragging;
    QPoint m_lastPanPoint;
    QPoint m_mapOffset;
    
    // Network and caching
    QNetworkAccessManager *m_networkManager;
    QHash<QString, TileInfo> m_tileCache;
    QTimer *m_updateTimer;
    
    // Location service
    QGeoPositionInfoSource *m_positionSource;
    
    // UI elements
    QVBoxLayout *m_layout;
    QHBoxLayout *m_controlsLayout;
    QComboBox *m_layerCombo;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QPushButton *m_resetButton;
    QPushButton *m_locateButton;
    QLabel *m_coordLabel;
    QSlider *m_zoomSlider;
    QWidget *m_mapArea;
    
    // Map settings
    static constexpr double DEFAULT_LATITUDE = 39.8283;  // Washington, DC
    static constexpr double DEFAULT_LONGITUDE = -98.5795; // Center of USA
    static constexpr int DEFAULT_ZOOM = 8;
    static constexpr int MIN_ZOOM = 1;
    static constexpr int MAX_ZOOM = 18;
    static constexpr int TILE_SIZE = 256;
};