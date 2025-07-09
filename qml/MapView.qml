import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Item {
    id: root
    
    property alias latitude: map.center.latitude
    property alias longitude: map.center.longitude
    property alias zoomLevel: map.zoomLevel
    property string activeMapType: "osm"
    
    function setCenter(lat, lon) {
        map.center = QtPositioning.coordinate(lat, lon)
    }
    
    function setZoom(zoom) {
        map.zoomLevel = zoom
    }
    
    function setMapLayer(layerType) {
        activeMapType = layerType
        updateMapType()
    }
    
    function updateMapType() {
        for (var i = 0; i < map.supportedMapTypes.length; i++) {
            var mapType = map.supportedMapTypes[i]
            if (activeMapType === "osm" && mapType.name.includes("OpenStreetMap")) {
                map.activeMapType = mapType
                return
            } else if (activeMapType === "satellite" && mapType.name.includes("Satellite")) {
                map.activeMapType = mapType
                return
            } else if (activeMapType === "terrain" && mapType.name.includes("Terrain")) {
                map.activeMapType = mapType
                return
            }
        }
        // Fallback to first available map type
        if (map.supportedMapTypes.length > 0) {
            map.activeMapType = map.supportedMapTypes[0]
        }
    }
    
    Plugin {
        id: mapPlugin
        name: "osm"
        
        PluginParameter {
            name: "osm.useragent"
            value: "GeoWorld 1.0"
        }
        
        PluginParameter {
            name: "osm.mapping.cache.directory"
            value: "/tmp/geoworld_map_cache"
        }
    }
    
    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        
        center: QtPositioning.coordinate(initialLatitude, initialLongitude)
        zoomLevel: initialZoom
        
        // Enable gestures
        gesture.enabled: true
        gesture.acceptedGestures: MapGestureArea.PanGesture | MapGestureArea.FlickGesture | MapGestureArea.PinchGesture
        
        // Map bounds
        minimumZoomLevel: 1
        maximumZoomLevel: 18
        
        Component.onCompleted: {
            updateMapType()
        }
        
        onCenterChanged: {
            if (mapWidget) {
                mapWidget.onMapCenterChanged()
            }
        }
        
        onZoomLevelChanged: {
            if (mapWidget) {
                mapWidget.onMapZoomChanged()
            }
        }
        
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            propagateComposedEvents: true
            
            onClicked: function(mouse) {
                var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                if (mapWidget) {
                    mapWidget.mapClicked(coord.latitude, coord.longitude)
                }
                mouse.accepted = false
            }
        }
        
        // Scale indicator
        Rectangle {
            id: scaleBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 10
            width: 100
            height: 20
            color: "white"
            border.color: "black"
            border.width: 1
            opacity: 0.8
            
            Text {
                anchors.centerIn: parent
                text: calculateScale()
                font.pixelSize: 12
                color: "black"
            }
            
            function calculateScale() {
                var metersPerPixel = 156543.03392 * Math.cos(map.center.latitude * Math.PI / 180) / Math.pow(2, map.zoomLevel)
                var meters = metersPerPixel * 100 // 100 pixels
                
                if (meters > 1000) {
                    return Math.round(meters / 1000) + " km"
                } else {
                    return Math.round(meters) + " m"
                }
            }
        }
        
        // Zoom controls
        Column {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            spacing: 2
            
            Rectangle {
                width: 30
                height: 30
                color: "white"
                border.color: "gray"
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: "+"
                    font.pixelSize: 16
                    font.bold: true
                }
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        map.zoomLevel = Math.min(map.zoomLevel + 1, map.maximumZoomLevel)
                    }
                }
            }
            
            Rectangle {
                width: 30
                height: 30
                color: "white"
                border.color: "gray"
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: "-"
                    font.pixelSize: 16
                    font.bold: true
                }
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        map.zoomLevel = Math.max(map.zoomLevel - 1, map.minimumZoomLevel)
                    }
                }
            }
        }
        
        // Attribution
        Text {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 5
            text: "© OpenStreetMap contributors"
            font.pixelSize: 10
            color: "gray"
        }
    }
}