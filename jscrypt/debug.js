Exec('deflib.js');
LoadModule('jscrypt');

//var cr = new Cipher("blowfish","test_key");
//cr.encrypt('123456');

//var h = new Hash("sha256");
//Print( h.hash('azuyvrzuivyozuiyvriouazyruioazvyrovazuiryvuioazyrvoazerhtoguizerhgtozeuigtoeurgtozeuriozeruighzerughzeoruithgozeiruhozeiruziruthgozeruizruhoziuzuivryuioazryvoazuivryoazuiryvioazuryuioazvryoazuivryuioazvryoazuivryoazuivryoazuivryuioazvryoazuivryoazuivryoazuivryuioazuvryoaziuvryazoevr').length );

var r = new Prng('yarrow', '546531s213584514');
r.AddEntropy(155651);

Print( r.name );