// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const jquery = require('jQuery');
const blobStream = require('blob-stream');
const merge = require('easy-pdf-merge');
const fs = require('fs');
const htmlToPdf = require('html-to-pdf');

//button
jquery('.make_pdf').on('click', function() {
    console.log('ji');
});



var html = '<p>Hi <b style="color: red;"> there </b> </p> <p> test </p>'

//conversion
 htmlToPdf.convertHTMLString(html, '/Users/schauhan/Downloads/bloodtest.pdf',
    function (error, success) {
        if (error) {
            console.log('Oh noes! Errorz!');
            console.log(error);
        } else {
            console.log('Woot! Success!');
            console.log(success);
        }
    }
);



//merge
merge(['/Users/schauhan/Downloads/bloodtest.pdf','/Users/schauhan/Downloads/patientdata.pdf'],'/Users/schauhan/Downloads/final.pdf',function(err){
 
        if(err)
        return console.log(err);
 
        console.log('Success');
 
});


