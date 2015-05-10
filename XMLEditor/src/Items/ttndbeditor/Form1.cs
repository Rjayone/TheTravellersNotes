using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace ttnItemsEditor
{
    public partial class Form1 : Form
    {
        List<Itm> Items = new List<Itm>();
        private XmlWriter wr;

        public Form1()
        {
            InitializeComponent();
            if (File.Exists("items.xml"))
            {
               
                XmlTextReader doc = new XmlTextReader("items.xml");
                doc.WhitespaceHandling = WhitespaceHandling.None;

                while (doc.Read())
                    if (doc.NodeType == XmlNodeType.Element)
                        if (doc.Name == "param")
                        {
                            Items.Add(new Itm(doc.GetAttribute("name"),doc.GetAttribute("descr"),doc.GetAttribute("size"),doc.GetAttribute("cost")));
                        }
                UpdateTable();
                doc.Close();
                UpdateXml();
            }
            else
            {
                UpdateXml();
            }
            
           

        }

        void Add(Itm it)
        {
            Items.Add(it);
            wr.WriteStartElement("param");
            wr.WriteAttributeString("name", it._name);
            wr.WriteAttributeString("descr", it._descr);
            wr.WriteAttributeString("size", it._size);
            wr.WriteAttributeString("cost", it._cost);
            wr.WriteEndElement();
            dataGridView1.Rows.Add(it._name, it._descr, it._size, it._cost);

        }

        void UpdateTable()
        {
            dataGridView1.Rows.Clear();
            foreach (var it in Items)
            {
                dataGridView1.Rows.Add(it._name, it._descr, it._size, it._cost);
            }
        }

        void UpdateXml()
        {
            try
            {
                wr.Close();
            }
            catch
            {
            }
            wr = new XmlTextWriter("items.xml", Encoding.Unicode);
            wr.WriteStartDocument();
            wr.WriteStartElement("items");
            foreach (var it in Items)
            {
                wr.WriteStartElement("param");
                wr.WriteAttributeString("name", it._name);
                wr.WriteAttributeString("descr", it._descr);
                wr.WriteAttributeString("size", it._size);
                wr.WriteAttributeString("cost", it._cost);
                wr.WriteEndElement();
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            wr.WriteEndElement();
            wr.WriteEndDocument();
            wr.Close();
        }

        private void dataGridView1_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            var itm = new Itm("", "", "", "");
            Add(itm);
        }

        private void dataGridView1_CellEndEdit(object sender, DataGridViewCellEventArgs e)
        {
            var Row = dataGridView1.Rows[e.RowIndex];
            Items[e.RowIndex] = new Itm(Row.Cells[0].Value.ToString(), Row.Cells[1].Value.ToString(), Row.Cells[2].Value.ToString(), Row.Cells[3].Value.ToString());
            UpdateXml();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (dataGridView1.SelectedCells.Count < 1)
                return;
            var index = dataGridView1.SelectedCells[0].RowIndex;
            Items.RemoveAt(index);
            UpdateTable();
            UpdateXml();
        }

        
    }
}
