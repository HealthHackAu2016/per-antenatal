var electron = require('electron').remote
var fs = require('fs');

var pdf = function(filename) {
    electron.getCurrentWebContents().printToPDF({}, function(err, data) {
        fs.writeFile(filename, data);
    })
}

module.exports = pdf
