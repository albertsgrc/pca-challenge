fs = require 'fs'

if process.argv.length < 3
	console.log "Required argument N"

N = process.argv[2]
MAX = if process.argv.length > 3 then process.argv[3] else 1000000
MIN = 1

string = ""

### To test some random
###
for i in [0...N]

	string += "#{Math.floor(Math.random() * (MAX - MIN)) + MIN}\n"
###
###

### To test all
###
#for i in [MIN..MAX]
#	string += "#{i}\n"
###
###

string += "0\n"

fs.writeFileSync('input', string)
