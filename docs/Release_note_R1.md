# Geo-location doesn't work
Unfortunately, geo-location can't be used.  Using the HTML5 geo-location API, the smartphone only allows to share the location with a secure website (HTTPS with a valid certificate).  We don't have a valid certificate.  Self-signed certificates are not accepted by the browser.

Another option would be to use a 3D-party geo-location service.  However, this would require an internet connection.  