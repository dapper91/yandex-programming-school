use ticks

var map =	function() {
				var interval_start = new Date(this.created)
				interval_start.setMinutes(Math.floor(this.created.getMinutes()/5)*5)
				interval_start.setSeconds(0)
				interval_start.setMilliseconds(0)

				price = parseInt(this.price)
				
				key = { 
					type : this.type, 
					symbol : this.symbol,
					interval_start : interval_start
				} 

				value = { 
					price : {
						open: price, 
						close: price,
						max: price,
						min: price,						
						sum: price
					},
					time : {
						open: this.created,
						close: this.created,
						min: this.created,
						max: this.created
					},
					count: 1
				}

				emit(key, value)
			};

var reduce = 	function(key, values) {
					var result = {
						price : {
							open: 0, 
							close: 0,
							max: 0,
							min: Number.MAX_VALUE,		
							sum: 0
						},
						time : {
							open: new Date(9999,0),
							close: new Date(0),
							min: 0,
							max: 0
						},
						count: 0
					};	

					values.forEach(function(value) {
						result.count += value.count;
						result.price.sum += value.price.sum;

						if (result.price.min > value.price.min)	{
							result.price.min = value.price.min;
							result.time.min = value.time.min;
						}
						if (result.price.max < value.price.max)	{
							result.price.max = value.price.max;
							result.time.max = value.time.max;
						}
						if (result.time.open > value.time.open)	{
							result.price.open = value.price.open;
							result.time.open = value.time.open;
						}
						if (result.time.close < value.time.close)	{
							result.price.close = value.price.close;
							result.time.close = value.time.close;
						}																
					});
					
					return result					
				};

var finalize = 	function(key, value) {					
					value.price.avg = value.price.sum/value.count;
					delete value["count"];
					delete value.price["sum"];

					return value											
				};

db.tmp_ticks.mapReduce(
		map,
		reduce,
		{			
			out: 'candlesticks',				
			finalize: finalize
		}
	)