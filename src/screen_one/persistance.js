 var fs = require('fs'); // Load the File System to execute our common tasks (CRUD)


  $.fn.serializeObject = function()
{
    var o = {};
    var a = this.serializeArray();
    $.each(a, function() {
        if (o[this.name] !== undefined) {
            if (!o[this.name].push) {
                o[this.name] = [o[this.name]];
            }
            o[this.name].push(this.value || '');
        } else {
            o[this.name] = this.value || '';
        }
    });
    return o;
};

function Save(){
    var fileName = "screen_one"; //TODO folder 
		var content = JSON.stringify($('form').serializeObject());
		 fs.writeFile(fileName, content, function (err) {
			 
           if(err){
               alert("An error ocurred creating the file "+ err.message)
           }
                        
           alert("The file has been succesfully saved");
       });
}

