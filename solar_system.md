## Sol.xml

This XML conforms to the Galaxy.xsd schema and defines the star system of Sol.
<pre><code>&lt;?xml version="1.0" encoding="utf-8" ?&gt;
&lt;StarSystem xmlns="http://schema.smallplanet.com/Galaxy" name="Solar System"&gt;
    
    &lt;!-- Sol --&gt;
    &lt;Star name="Sol" age="4570000" mass="3330000" equatorialRadius="109" equatorialGraviry="28"/&gt;
    
    &lt;!-- Sol's Planets --&gt;
	&lt;Planet name="Mercury" mass="0.055"/&gt;
	&lt;Planet name="Venus" mass="0.815"/&gt;
	&lt;Planet name="Earth" mass="1"&gt;
		&lt;Moon name="Moon" mass="0.0123"/&gt;
	&lt;/Planet&gt;
	&lt;Planet name="Mars" mass="0.107"/&gt;
	&lt;Planet name="Jupiter" mass="318"&gt;
		&lt;Moon name="Io"/&gt;
		&lt;Moon name="Europa"/&gt;
		&lt;Moon name="Ganymede"/&gt;
		&lt;Moon name="Callisto"/&gt;
	&lt;/Planet&gt;
	&lt;Planet name="Saturn" mass="95" &gt;
		&lt;Moon name="Mimas"/&gt;
		&lt;Moon name="Enceladus"/&gt;
		&lt;Moon name="Tethys"/&gt;
		&lt;Moon name="Dione"/&gt;
		&lt;Moon name="Rhea"/&gt;
		&lt;Moon name="Titan"/&gt;
		&lt;Moon name="Iapetus"/&gt;
	&lt;/Planet&gt;
	&lt;Planet name="Uranus" mass="14"&gt;
		&lt;Moon name="Miranda"/&gt;
		&lt;Moon name="Ariel"/&gt;
		&lt;Moon name="Umbriel"/&gt;
		&lt;Moon name="Titania"/&gt;
		&lt;Moon name="Oberon"/&gt;
	&lt;/Planet&gt;
	&lt;Planet name="Neptune" mass="17"&gt;
		&lt;Moon name="Triton"/&gt;
	&lt;/Planet&gt;
    
&lt;/StarSystem&gt;
</code></pre>
