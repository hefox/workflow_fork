-- --------------------------------------------------------

--
-- Table structure for table `workflow_states`
--

CREATE TABLE workflow_states (
  sid integer NOT NULL default '0',
  wid integer NOT NULL default '0',
  state varchar(255) NOT NULL default '',
  weight smallint NOT NULL default '0',
  sysid smallint NOT NULL default '0',
  PRIMARY KEY  (sid)
);

CREATE INDEX workflow_states_wid ON workflow_states USING HASH (wid);
CREATE INDEX workflow_states_sysid ON workflow_states USING HASH (sysid);

-- --------------------------------------------------------

--
-- Table structure for table `workflow_transitions`
--

CREATE TABLE workflow_transitions (
  tid integer NOT NULL default '0',
  sid integer NOT NULL default '0',
  target_sid integer NOT NULL default '0',
  PRIMARY KEY  (tid)
);
 
CREATE INDEX workflow_transitions_sid ON workflow_transitions USING HASH (sid);
CREATE INDEX workflow_transitions_target_sid ON workflow_transitions USING HASH (target_sid);

-- --------------------------------------------------------

--
-- Table structure for table `workflows`
--

CREATE TABLE workflows (
  wid integer NOT NULL default '0',
  name varchar(255) NOT NULL default '',
  PRIMARY KEY  (wid)
);

-- --------------------------------------------------------

--
-- Table structure for table `workflow_type_map`
--

CREATE TABLE workflow_type_map (
  type varchar(255) NOT NULL default '',
  wid integer NOT NULL default '0'
);
  
CREATE INDEX workflow_type_map_type ON workflow_type_map (type,wid);

-- --------------------------------------------------------

--
-- Table structure for table `workflow_node`
--

CREATE TABLE workflow_node (
  nid integer NOT NULL default '0',
  sid integer NOT NULL default '0'
);

CREATE INDEX workflow_node_nid ON workflow_node (nid,sid);

-- --------------------------------------------------------

--
-- Table structure for table `workflow_actions`
--

CREATE TABLE workflow_actions (
  tid integer NOT NULL default '0',
  aid varchar(255) NOT NULL default '0',
  weight integer NOT NULL default '0'
);

CREATE INDEX workflow_actions_tid ON workflow_actions USING HASH (tid);

