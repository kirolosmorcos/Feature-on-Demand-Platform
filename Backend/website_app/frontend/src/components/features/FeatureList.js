import React, { useEffect, useMemo, useState, useCallback } from "react";
import { Link } from "react-router-dom";
import featureService from "../../services/featureService";
import Loading from "../common/Loading";


function FeatureList({ showAlert }) {

    /* =========================
        STATE
    ========================== */

    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const [features, setFeatures] = useState([]);
    const [searchTerm, setSearchTerm] = useState("");
    const [confirmDeleteCode, setConfirmDeleteCode] = useState(null);

    /* =========================
        FETCH DATA
    ========================== */

    const fetchFeatures = useCallback(async () => {
        try {
            setLoading(true);
            setError(null);

            const data = await featureService.getAllFeatures();
            setFeatures(Array.isArray(data) ? data : []);
        } catch (err) {
            console.error("Failed to fetch features:", err);
            setError("Unable to load features. Please try again.");
            showAlert?.("error", "Failed to fetch features");
        } finally {
            setLoading(false);
        }   
    }, [showAlert]);

    useEffect(() => {
        fetchFeatures();
    }, [fetchFeatures]);

    /* =========================
        SEARCH (Derived State)
    ========================== */

    const filteredFeatures = useMemo(() => {
        const term = searchTerm.trim().toLowerCase();
        if (!term) return features;

        return features.filter(
            (feature) =>
                feature.feature_name.toLowerCase().includes(term) ||
                feature.feature_code.toLowerCase().includes(term)
        );
    }, [features, searchTerm]);

    /* =========================
        DELETE HANDLER
    ========================== */

    const handleDelete = async (featureCode) => {
        if (confirmDeleteCode !== featureCode) {
            setConfirmDeleteCode(featureCode);
            return;
        }
        try {
            await featureService.deleteFeature(featureCode);
            showAlert?.("success", `Feature ${featureCode} deleted successfully`);
            fetchFeatures(); // Refresh the list
        } catch (err) {
            console.error("Failed to delete feature:", err);
            showAlert?.("error", `Failed to delete feature ${featureCode}`);
        }   finally {
            setConfirmDeleteCode(null);
        }
    };

    /* =========================
        RENDER
    ========================== */

    if (loading) {
        return <Loading message="Loading features..." />;
    }

    if (error) {
        return <div className="alert alert-danger">{error}</div>;
    }


   return (
       <div className="feature-list">
         <div className="d-flex justify-content-between align-items-center pt-3 pb-2 mb-3 border-bottom">
           <h1 className="h2">Features</h1>
   
           <Link to="/features/new" className="btn btn-sm btn-primary">
             <i className="bi bi-plus-circle me-1"></i>
             Add Feature
           </Link>
         </div>
   
         {/* Search */}
         <div className="mb-3">
           <input
             type="text"
             className="form-control"
             placeholder="Search by name or feature code..."
             value={searchTerm}
             onChange={(e) => setSearchTerm(e.target.value)}
           />
         </div>
   
         {/* Empty State */}
         {filteredFeatures.length === 0 ? (
           <div className="alert alert-info">
             {searchTerm
               ? "No features match your search."
               : "No features available. Add one to get started."}
           </div>
         ) : (
           <div className="table-responsive">
             <table className="table table-striped table-hover align-middle">
               <thead>
                 <tr>
                   <th>Feature Name</th>
                   <th>Feature Code</th>
                   <th style={{ width: 150 }}>Actions</th>
                 </tr>
               </thead>
               <tbody>
                 {filteredFeatures.map((feature) => (
                   <tr key={feature.feature_code}>
                     <td>{feature.feature_name}</td>
                     <td className="text-muted">{feature.feature_code}</td>
                     <td>
                       <div className="btn-group">
   
                         <Link
                           to={`/features/${feature.feature_code}`}
                           className="btn btn-sm btn-outline-info"
                         >
                           <i className="bi bi-eye"></i>
                         </Link>
   
                         <Link
                           to={`/features/edit/${feature.feature_code}`}
                           className="btn btn-sm btn-outline-primary"
                         >
                           <i className="bi bi-pencil"></i>
                         </Link>
   
                         <button
                           className={`btn btn-sm ${
                             confirmDeleteCode === feature.feature_code
                               ? "btn-danger"
                               : "btn-outline-danger"
                           }`}
                           onClick={() =>
                             handleDelete(feature.feature_code)
                           }
                         >
                           {confirmDeleteCode === feature.feature_code
                             ? "Confirm"
                             : <i className="bi bi-trash"></i>}
                         </button>
   
                       </div>
                     </td>
                   </tr>
                 ))}
               </tbody>
             </table>
           </div>
         )}
       </div>
     );
   }
   
   export default FeatureList;