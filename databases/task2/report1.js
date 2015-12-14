use rent

var mapAdvs =	function() {
					emit( this.owner_id, { count: 1 } )
				};

var mapAgents =	function() {
					emit( this._id, {fullname: this.fullname, phone: this.phone } )
				};

var reduceAdvs =	function(key, values) {	
						var count = 0;	
						values.forEach(function(value) {						
							count += value.count;						
						});				
						return { count: count };
					};

var reduceJoin =	function(key, values) {	
						var result = {count: 0}

					    values.forEach(function(value) {					   
						    if('fullname' in value) 
						    	result.fullname = value.fullname;
						    if('phone' in value) 
						    	result.phone = value.phone
						    if('count' in value) 
						    	result.count += value.count
					    });

					    return result;
					};

//counting
db.advs.mapReduce(
		mapAdvs, 
		reduceAdvs, 
		{
			out: 'tmp_agents'
		}
	);

//join
db.agents.mapReduce(
		mapAgents, 
		reduceJoin, 
		{
			out: {reduce : 'tmp_agents'}
		}
	);


print("имя;телефон;кол-во предложений")
db.tmp_agents.find( { 'value.count': { $gt: 1 } } ).sort( { 'value.count' : -1 } ).forEach(function(item) {		
	print(item.value.fullname + ';' + item.value.phone + ';' + item.value.count);
});

db.tmp_agents.drop()