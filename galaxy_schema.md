## Galaxy.xsd

This schema defines the structure of our Galaxy data model.
<pre><code>&lt;schema xmlns="http://www.w3.org/2001/XMLSchema"
    xmlns:ga="http://schema.smallplanet.com/Galaxy"
    targetNamespace="http://schema.smallplanet.com/Galaxy"&gt;
    
    &lt;!-- Star System --&gt;
    &lt;element name="StarSystem" type="ga:StarSystem"/&gt;
    &lt;complexType name="StarSystem"&gt;
    	&lt;sequence&gt;
            &lt;element ref="ga:Planet" minOccurs="0" maxOccurs="unbounded" /&gt;
        &lt;/sequence&gt;
    &lt;/complexType&gt;
	
    &lt;!-- Astronomical Object --&gt;
    &lt;element name="AstronomicalObject" type="ga:AstronomicalObject"/&gt;
    &lt;complexType name="AstronomicalObject"&gt;
        &lt;attribute name="name" type="string" use="required" /&gt;
        &lt;attribute name="mass" type="float" use="optional" /&gt;
    &lt;/complexType&gt;
    
    &lt;!-- Planet --&gt;
    &lt;element name="Planet" type="ga:Planet"/&gt;
    &lt;complexType name="Planet"&gt;
        &lt;complexContent&gt;
            &lt;extension base="ga:AstronomicalObject"&gt;
                &lt;sequence&gt;
                    &lt;element ref="ga:Moon" minOccurs="0" maxOccurs="unbounded" /&gt;
                &lt;/sequence&gt;
                &lt;attribute name="hasRings" type="boolean" default="false" /&gt;
            &lt;/extension&gt;
        &lt;/complexContent&gt;
    &lt;/complexType&gt;
    
    &lt;!-- Moon --&gt;
    &lt;element name="Moon" type="ga:Moon"/&gt;
    &lt;complexType name="Moon"&gt;
        &lt;complexContent&gt;
            &lt;extension base="ga:AstronomicalObject"/&gt;
        &lt;/complexContent&gt;
    &lt;/complexType&gt;
    
&lt;/schema&gt;
</code></pre>
