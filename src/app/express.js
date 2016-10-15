/**
 * Express API to render posted html to pdf files.
 */
const express = require('express');
const jquery = require('jquery');
const merge = require('easy-pdf-merge');
const htmlToPdf = require('html-to-pdf');
const bodyParser = require('body-parser');
const mustache = require('mustache');
const expMustache = require('mustache-express')();
const fs = require('fs');

// create the app
var app = express();

// set up our static assets
app.use(express.static('bower_components/bootstrap/dist/'));
app.use('/js', express.static('bower_components/jquery-1.9.1/'));

// setup request body json parsing
app.use(bodyParser.urlencoded({extended: false}));

// setup mustache template engine
expMustache.cache = null; // disable cache
app.engine('html', expMustache);
app.set('views', './src/templates');
app.set('view engine', 'mustache');

// posting html content to this endpoint will save it to a pdf
app.route('/page-:pageNum')
    .get(function (req, res) { // return the rendered template
        res.render('screen'+req.params.pageNum+'.html');
    })
    .post(function (req, res, next) {
        console.log(req.body);
        // var contents = fs.readFileSync('./src/templates/screen'+req.params.pageNum+'.html','utf8');
        // htmlToPdf.convertHTMLString(mustache.render(contents, req.body), 'page'+req.params.pageNum+'.pdf');
        // render the next template
        res.redirect('/page-'+(+req.params.pageNum+1));
    });


// export the app for electron to require
module.exports = app;
