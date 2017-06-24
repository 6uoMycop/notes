## A "Hello World" JavaServer Page
JavaServer Pages (JSPs) are, in the most basic sense, Web pages with embedded Java code. The embedded Java code is executed on the server before the page is returned to the browser.
```
<html>
<body>
<%
    out.println("<h1>Hello World!</h1>");
%>
</body>
</html>>
```
As you can see, the page is composed of standard HTML with a dash of Java contained between the <% and %> character sequences. The <% and %> along with a code inside is called a script.

If you want to try out the "Hello World" JSP, put it in the same place that you would put an HTML page and point your browser to it. You need to install a JSP-capable Web server, such as Tomcat. If you already have an installation of Tomcat, you can save this example as a file named HelloWorld.jsp and place it in the webapps/ROOT folder located in your TOmcat installation directory. Then enter http://localhost:8080/HelloWorld.jsp into your browser as the address.