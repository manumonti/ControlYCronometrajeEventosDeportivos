var curve = require('curve25519-n');
var nacl = require('tweetnacl');
nacl.util = require('tweetnacl-util');
var crypto = require('crypto');
var HashMap = require('hashmap');


var random;

var keys = new HashMap();


function randomValueHex (len) {
    return crypto.randomBytes(Math.ceil(len/2))
        .toString('hex') // convert to hexadecimal format
        .slice(0,len);   // return required number of characters
}


// required when running on node.js
var mqtt = require('mqtt');

var mysecret = new Buffer.from('0070FE2A16EE384CC25D84DEE53718B2D655545872CEEC7ED57F12F1EE8E5E59', 'hex');
var hispublic = new Buffer.from('0F767556C7A9576DC1D3B13FC219CC9E5DF8F3082CDFD394DB64DEFE68E1BC7D', 'hex');

var sk1 = hexStringToByte('10343c54e2595d2b613f50d9c6ae3958c1f5ffb2aa0882708159d383aa8eb24e');
var pk1 = hexStringToByte('a6210f6a4cd28a83986cb2284d4aa3e41986eef157bcf7e3b9e09c8bfe8f2b63');

var sk2 = hexStringToByte('001eb892282978f30573dd3116b48fc0efe3cb37cc2d7b9c136340572a74d943');
var pk2 = hexStringToByte('0e41c8ce8aff43be66b9538f99445aeae1778a03e53c7d4584c83c85e27a3508');

var client = mqtt.connect('mqtt://72709a72:89944f50c966485e@broker.shiftr.io', {
  clientId: 'javascript'
});

client.on('connect', function(){
  console.log('client has connected!');

  client.subscribe('/public_dh/+');
  client.subscribe('/hmac/+');
  // client.unsubscribe('/example');


  setInterval(function(){
  	random = randomValueHex(32) // value 'd5be8583137b'
    client.publish('/challenge', random);
  }, 5000);
});

client.on('message', function(topic, message) {
  console.log('new message:', topic, message.toString());
  if (topic.substr(0,10) == "/public_dh")
  {
  	var name = topic.substr(11,topic.length);
  	console.log('New Public Key from '+ name +" : "+ message.toString());
  	var jout1 = nacl.scalarMult(sk1, hexStringToByte(message.toString()));
	console.log (byteToHexString(jout1));
	keys.set(name, byteToHexString(jout1));
	//var jout2 = nacl.scalarMult(sk2, hexStringToByte(message.toString()));
	//console.log (byteToHexString(jout2));
  } else if (topic.substr(0,5) == "/hmac")
  {
  	var name = topic.substr(6,topic.length);
  	console.log('New HMAC from '+ name +" : "+ message.toString());

	const hmac = crypto.createHmac('sha256', Buffer.from(keys.get(name), 'hex'));
	hmac.update(Buffer.from(random, 'hex'));
  	console.log('Local HMAC  ('+ name +") : "+ hmac.digest('hex'));
  	console.log('Challenge : '+ random);

  }
});

function byteToHexString(uint8arr) {
  if (!uint8arr) {
    return '';
  }
  
  var hexStr = '';
  for (var i = 0; i < uint8arr.length; i++) {
    var hex = (uint8arr[i] & 0xff).toString(16);
    hex = (hex.length === 1) ? '0' + hex : hex;
    hexStr += hex;
  }
  
  return hexStr.toUpperCase();
}

function hexStringToByte(str) {
  if (!str) {
    return new Uint8Array();
  }
  
  var a = [];
  for (var i = 0, len = str.length; i < len; i+=2) {
    a.push(parseInt(str.substr(i,2),16));
  }
  
  return new Uint8Array(a);
}


// My IP address: 192.168.2.22.
// Generating random k/f for Alice ...
// Alice Public (k): a6210f6a4cd28a83986cb2284d4aa3e41986eef157bcf7e3b9e09c8bfe8f2b63
// 0xA6, 0x21, 0x0F, 0x6A, 0x4C, 0xD2, 0x8A, 0x83, 0x98, 0x6C, 0xB2, 0x28, 0x4D, 0x4A, 0xA3, 0xE4, 0x19, 0x86, 0xEE, 0xF1, 0x57, 0xBC, 0xF7, 0xE3, 0xB9, 0xE0, 0x9C, 0x8B, 0xFE, 0x8F, 0x2B, 0x63, 
// Alice Private (f): 10343c54e2595d2b613f50d9c6ae3958c1f5ffb2aa0882708159d383aa8eb24e
// Elapsed time (in microseconds): 27442

// Generating random k/f for Bob ...
// Bob Public (K): 0e41c8ce8aff43be66b9538f99445aeae1778a03e53c7d4584c83c85e27a3508
// 0x0E, 0x41, 0xC8, 0xCE, 0x8A, 0xFF, 0x43, 0xBE, 0x66, 0xB9, 0x53, 0x8F, 0x99, 0x44, 0x5A, 0xEA, 0xE1, 0x77, 0x8A, 0x03, 0xE5, 0x3C, 0x7D, 0x45, 0x84, 0xC8, 0x3C, 0x85, 0xE2, 0x7A, 0x35, 0x08, 
// Bob Private (f): 001eb892282978f30573dd3116b48fc0efe3cb37cc2d7b9c136340572a74d943
// Elapsed time (in microseconds): 28260

// Generating shared secret for Alice ...
// Alice shared key: 5f920e27eeac646e5dc38d5469563b31c82b3caf656cfb38069fccf6b1ce1f33
// Elapsed time (in microseconds): 28543

// Generating shared secret for Bob ...
// Bob shared key: 5f920e27eeac646e5dc38d5469563b31c82b3caf656cfb38069fccf6b1ce1f33
// Elapsed time (in microseconds): 28232

// Conectado



var result = new Buffer.alloc(32);

//let hex = Buffer.from(uint8).toString('hex');


var jout1 = nacl.scalarMult(sk1, pk2);

console.log (byteToHexString(jout1));
 
var jout2 = nacl.scalarMult(sk2, pk1);

console.log (byteToHexString(jout2));

result = curve.deriveSharedSecret(sk2, pk1)

//result = curve.curve(buf1,secretKeyAlice,expectedPublicKeyBob);

console.log(result.toString('hex'));

result = curve.deriveSharedSecret(sk1, pk2)

console.log(result.toString('hex'));
