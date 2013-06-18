
import java.io.*;
import java.sql.*;
import java.net.*;
/**
 * Describe class RemoteServer here.
 *
 *
 * Created: Wed Aug  8 11:05:09 2012
 *
 * @author Cynthia Jing
 * @version 1.0
 */
public class RemoteServer{
  static{
    System.loadLibrary("blackadder_java");
  }
  static final blackadder_java.BA BA=null;
  static blackadder_java.Blackadder ba =
	    blackadder_java.Blackadder.Instance(true);
  protected int listenPort = 3000;
  //private Connection con= Setup.connection("userdatabase");
  
  public static void main(String[] args) throws Exception{
    RemoteServer server = new RemoteServer();
  

    byte[] prefix_id = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte[] id = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

    byte[] full_id = new byte[prefix_id.length + id.length];
    System.arraycopy(prefix_id, 0, full_id, 0, prefix_id.length);
    System.arraycopy(id, 0, full_id, prefix_id.length, id.length);
    ba.subscribe_scope(id,prefix_id,BA.NODE_LOCAL, null);	
    server.acceptConnections();

  }

  public void acceptConnections(){
    try{
      ServerSocket server = new ServerSocket(listenPort);
      Socket incomingConnection = null;
      while (true){
	incomingConnection = server.accept();
	//	handleConnection(incomingConnection);
	getBlackadder(incomingConnection);
      }
    }catch (BindException e){
      System.out.println("Unable to bind to port: " + listenPort);
    }catch (IOException e){
      System.out.println("Unable to start a ServerSocket on port: " + listenPort);
    }
    System.out.println("Server acceptConnections end");
  }

  public void handleConnection(Socket incomingConnection){
    // connect to program that sends/receives info from dirctory

    /* try{
      OutputStream outputToSocket = incomingConnection.getOutputStream();
      InputStream inputFromSocket = incomingConnection.getInputStream();
      BufferedReader streamReader = new BufferedReader(new InputStreamReader(inputFromSocket));
      PrintWriter streamWriter = new PrintWriter(outputToSocket);
      String query = streamReader.readLine();
      
      PreparedStatement statement = con.prepareStatement(query);
      ResultSet result = statement.executeQuery();

      ResultSetMetaData rsmd = result.getMetaData();
      int numberOfColumns = rsmd.getColumnCount();

      while (result.next()){
	String col = result.getString(1);
	if (numberOfColumns>1){
	  for (int i=2; i<=numberOfColumns; i++){
	    col += " ";
	    col += result.getString(i);
	  }
	}
	streamWriter.println(col);
	streamWriter.flush();
      }
      streamWriter.close();
      streamReader.close();
    }catch (Exception e){
      System.out.println("Error handling a client: " + e);
      }*/
  }
  public void getBlackadder(Socket incomingConnection){
     byte[] sid = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
     byte[] rid = { 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 };
     byte[] target_id = new byte[sid.length +rid.length];
     System.arraycopy(sid, 0, target_id, 0, sid.length);
     System.arraycopy(rid, 0, target_id, sid.length, rid.length);
     
     try{
       OutputStream outputToSocket = incomingConnection.getOutputStream();
       InputStream inputFromSocket = incomingConnection.getInputStream();
       BufferedReader streamReader = new BufferedReader(new InputStreamReader(inputFromSocket));
       PrintWriter streamWriter = new PrintWriter(outputToSocket);
       String query = streamReader.readLine();
     
       //  System.Text.Encoding enc = System.Text.Encoding.ASCII;
       byte[] data = query.getBytes("UTF-8");

       ba.publish_data(target_id, BA.NODE_LOCAL, null, data);
     
       boolean wait = true;
       while (wait) {
	 blackadder_java.Event ev = new blackadder_java.Event();
	 ba.getEvent(ev);
	 int type = ev.getType();
	 System.out.println(type);
	 if (type == 0)
	   break;
	 if (type == BA.PUBLISHED_DATA) {
	   byte[] data2 = ev.getData();
	   System.out.println(ev.getData_len());
	   System.out.println(new String(data2)); 
	   streamWriter.println(data2);
	   streamWriter.flush();
	   wait = false;
	 }
       }
       streamWriter.close();
       streamReader.close();
     }catch (Exception e){
       System.out.println("Error handling a client: " + e);
     }
  }
} 