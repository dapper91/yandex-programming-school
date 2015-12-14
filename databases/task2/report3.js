use rent

var map =	function() {
				key = this.district;
				value = { flats: [ { id: this._id, price: this.price } ] }

				emit(key, value)				
			};


var reduce =	function(key, values) {	
					result = { flats: [] }

					for (var i = 0; i < values.length; i++) {
						result.flats.push.apply(result.flats, values[i].flats);						
					}

					result.flats.sort( function sortFunction(a, b) {
									return a.price - b.price								
							    }
					)	

				    return result
				};


db.advs.mapReduce(
		map,
		reduce,
		{
			out: 'tmp_bestadvs',	
			query: { $and: [ {rooms_number: 3}, {district: {$exists: true} } ] }			
		}
	)


var proposals = 5;


var head = "район;";
for (var i = 0; i < proposals; i++) {
	var head = head.concat("предложение №" + (i + 1) + ";");
}
print(head)

db.tmp_bestadvs.find().forEach(function(item) {
	var row = item._id + ';';

	for (var i = 0; i < proposals; i++) {
		var flat = item.value.flats[i];

		row += (flat === undefined) ? "-/-;" : (flat.id + '/' + flat.price + ';');
		
	}
	print(row);
});

db.tmp_bestadvs.drop()