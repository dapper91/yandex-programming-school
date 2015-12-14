use rent

var map =	function() {
				key = parseInt(this.area_details['общая:'])
				value = {sum: this.price, count: 1, min: this.price, max: this.price}

				emit(key, value)
			};

var reduce = 	function(key, values) {
					var sum = 0;
					var count = 0;
					var min = Number.MAX_VALUE;
					var max = 0;				

					values.forEach(function(value) {
						sum += value.sum;
						count += value.count;
						min = Math.min(min, value.min); 
						max = Math.max(max, value.max); 						
					});
					
					return { sum: sum, count: count, min: min, max: max};
				};

var finalize = 	function(key, value) {
					var avg = Math.round(value.sum/value.count);
					return {avg: avg, min: value.min, max: value.max}					
				};

db.advs.mapReduce(
		map,
		reduce,
		{
			out: 'tmp_prices',	
			query: { 'area_details.общая:': { $exists: true } },
			finalize: finalize
		}
	)

print("метраж,минимальная цена;максимальная цена;средняя цена")
db.tmp_prices.find().sort( {_id : 1} ).forEach(function(item) {
	print(item._id + ';' + item.value.min + ';' + item.value.max + ';' + item.value.avg);
});

db.tmp_prices.drop()